#include <SqlDbLib/Parser.h>

#include <cstring>
#include <cstdio>
#include <cctype>

#include <unordered_map>
#include <vector>
#include <deque>
#include <list>
#include <string>
#include <algorithm>
#include <memory>
#include <filesystem>
#include <sstream>

#include "ParserPrivate.h"
#include "AutogenParser.h"

static const std::unordered_map<std::string, SqlKeywordType> keyword_map = {
	{"AND", SQL_KEYWORD_TYPE_AND},
	{"OR", SQL_KEYWORD_TYPE_OR},
	{"NOT", SQL_KEYWORD_TYPE_NOT},
	{"TRUE", SQL_KEYWORD_TYPE_TRUE},
	{"FALSE", SQL_KEYWORD_TYPE_FALSE},
	{"WHERE", SQL_KEYWORD_TYPE_WHERE},
	{"ORDER", SQL_KEYWORD_TYPE_ORDER},
	{"BY", SQL_KEYWORD_TYPE_BY},
	{"SELECT", SQL_KEYWORD_TYPE_SELECT},
	{"INSERT", SQL_KEYWORD_TYPE_INSERT},
	{"UPDATE", SQL_KEYWORD_TYPE_UPDATE},
	{"DELETE", SQL_KEYWORD_TYPE_DELETE},
	{"FROM", SQL_KEYWORD_TYPE_FROM},
	{"INTO", SQL_KEYWORD_TYPE_INTO},
	{"VALUES", SQL_KEYWORD_TYPE_VALUES},
	{"SET", SQL_KEYWORD_TYPE_SET},
};

/**/

struct SqlParseTree
{
	virtual ~SqlParseTree() = default;
};

struct SqlParseTree_Condition : public SqlParseTree
{
	enum class ComparisonOperator
	{
		NONE = 0,
		EQUAL,
		NOT_EQUAL,
		LESS,
		LESS_OR_EQUAL,
		GREATER,
		GREATER_OR_EQUAL,
	};

	enum class ConjunctionOperator
	{
		NONE = 0,
		AND,
		OR,
	};

	std::string column;
	SqlToken *value {nullptr};
	ComparisonOperator op {ComparisonOperator::NONE};

	SqlParseTree_Condition *right {nullptr};
	ConjunctionOperator conj_op {ConjunctionOperator::NONE};

	~SqlParseTree_Condition() override
	{
		delete right;
	}
};

struct SqlParseTree_WhereClause : public SqlParseTree
{
	SqlParseTree_Condition *condition {nullptr};

	~SqlParseTree_WhereClause() override
	{
		delete condition;
	}
};

struct SqlParseTree_Select : public SqlParseTree
{
	bool all_columns {false};
	std::list<std::string> columns;
	std::list<std::string> tables;
	SqlParseTree_WhereClause *where {nullptr};

	~SqlParseTree_Select() override
	{
		delete where;
	}
};

/**/

SqlAstNode *SqlParser_Parse(const char *sql, size_t size)
{
	// Tokenize input
	std::vector<SqlToken> tokens;
	tokens.reserve(32);
	tokens.emplace_back();
	SqlToken *current_token = &tokens.back();

	for (size_t i = 0; i < size; i++)
	{
		char c = sql[i];
		char nc = (i + 1 < size) ? sql[i + 1] : '\0';

		if (c == '\0')
		{
			break;
		}
		if (isspace(c))
		{
			continue;
		}
		else if (c == '-' && nc == '-')
		{
			while (i++ < size && sql[i] != '\n');
		}
		else if (c == '_' || isalpha(c))
		{
			current_token->type = SQL_TOKEN_TYPE_IDENTIFIER;
			current_token->buffer = &sql[i];
			size_t buffer_size = i - 1;
			while (i++ < size)
			{
				c = sql[i];
				if (c != '_' && !isalpha(c) && !isdigit(c))
				{
					--i;
					break;
				}
			}
			current_token->size = i - buffer_size;

			char identifier[256];
			memcpy(identifier, current_token->buffer, current_token->size);
			identifier[current_token->size] = '\0';
			std::transform(identifier, identifier + current_token->size, identifier, ::toupper);
			if (keyword_map.count(identifier))
			{
				current_token->type = SQL_TOKEN_TYPE_KEYWORD;
				current_token->keyword_type = keyword_map.at(identifier);

				if (strncmp(identifier, "TRUE", 4) == 0 || strncmp(identifier, "FALSE", 5) == 0)
				{
					current_token->type = SQL_TOKEN_TYPE_BOOLEAN;
				}
			}

			tokens.emplace_back();
			current_token = &tokens.back();
		}
		else if (isdigit(c))
		{
			current_token->type = SQL_TOKEN_TYPE_NUMBER;
			current_token->buffer = &sql[i];
			size_t buffer_size = i - 1;
			while (i++ < size)
			{
				c = sql[i];
				if (!isdigit(c) && c != '.')
				{
					--i;
					break;
				}
			}
			current_token->size = i - buffer_size;

			tokens.emplace_back();
			current_token = &tokens.back();
		}
		else if (c == '"')
		{
			current_token->type = SQL_TOKEN_TYPE_STRING;
			current_token->buffer = &sql[i];
			size_t buffer_size = i - 1;
			while (i++ < size)
			{
				c = sql[i];
				if (c == '"')
				{
					// Go past the quote
					++i;
					break;
				}
			}
			current_token->size = i - buffer_size;

			tokens.emplace_back();
			current_token = &tokens.back();
		}
		else if (ispunct(c))
		{
			current_token->type = SQL_TOKEN_TYPE_OPERATOR;
			current_token->buffer = &sql[i];
			current_token->size = 1;

			tokens.emplace_back();
			current_token = &tokens.back();
		}
		else
		{
			current_token->type = SQL_TOKEN_TYPE_UNKNOWN;
			current_token->buffer = &sql[i];
			current_token->size = 1;

			tokens.emplace_back();
			current_token = &tokens.back();
		}
	}
	if (tokens.back().type == SQL_TOKEN_TYPE_UNKNOWN)
		tokens.pop_back();

	for (const SqlToken &token : tokens)
	{
		const char *type = [token](){
			switch (token.type)
			{
				case SQL_TOKEN_TYPE_UNKNOWN: return "UNKN";
				case SQL_TOKEN_TYPE_KEYWORD: return "KEYW";
				case SQL_TOKEN_TYPE_IDENTIFIER: return "IDNF";
				case SQL_TOKEN_TYPE_STRING: return "STRG";
				case SQL_TOKEN_TYPE_NUMBER: return "NUMB";
				case SQL_TOKEN_TYPE_OPERATOR: return "OPER";
				case SQL_TOKEN_TYPE_BOOLEAN: return "BOOL";
			}
			return "";
		}();
//		printf("%s: %.*s\n", type, (int)token.size, token.buffer);
	}

	std::deque<SqlToken *> q;
	for (SqlToken &token : tokens)
		q.push_back(&token);

	return SqlParseTree_Parse_Root(q);
}

void SqlParse_DestroyParseTree(SqlAstNode *tree)
{
	delete tree;
}

void SqlParser_PrintParseTree(SqlAstNode *tree)
{
	printf("SqlParser_PrintParseTree: Unimplemented\n");
}

static void TraverseTree(SqlAstNode *node, std::stringstream &ss, int level = 0, bool last = true)
{
	ss << "{";
	ss << R"("type":")" << node->type << "\",";
	std::string value = node->value;
	std::transform(value.begin(), value.end(), value.begin(), [](char c) { return c == '"' ? '\'' : c; });
	ss << R"("value":")" << value << "\",";
	ss << R"("children":[)";

	size_t num_children = node->children.size();
	size_t i = 0;
	for (auto child : node->children)
	{
		TraverseTree(child, ss, level + 1, i == num_children - 1);
		i++;
	}
	ss << "]}";
	if (!last)
		ss << ",";
}

void SqlParser_DumpTree(SqlAstNode *tree, const char *sql_raw, size_t sql_raw_len)
{
	std::filesystem::path repo_path = SQLDB_REPO_PATH;
	std::filesystem::path graph_path = repo_path / "intermediate" / "graph.json";
	std::filesystem::create_directories(graph_path.parent_path());

	std::stringstream ss;
	std::string sql(sql_raw, sql_raw_len);
	std::transform(sql.begin(), sql.end(), sql.begin(), [](char c) { return c == '"' ? '\'' : c; });

	ss << "{";
	ss << "\"raw\": \"" << sql << "\"";
	ss << ", ";
	ss << "\"tree\": ";
	TraverseTree(tree, ss);
	ss << "}";

	FILE *f = fopen(graph_path.string().c_str(), "w");
	fprintf(f, "%s\n", ss.str().c_str());
	fclose(f);
}
