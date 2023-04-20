#pragma once
#include <SqlDbLib/Export.h>

#include <cstdint>
#include <string> // TODO: Remove
#include <list> // TODO: Remove

struct SqlAstNode
{
	const char *type;
	std::string value;

	SqlAstNode *parent;
	std::list<SqlAstNode *> children;

	~SqlAstNode()
	{
		for (auto child : children)
			delete child;
	}
};

inline const SqlAstNode SqlAstNode_Empty{};

SQLDBLIB_EXPORT SqlAstNode *SqlParser_Parse(const char *sql, size_t size);
SQLDBLIB_EXPORT void SqlParse_DestroyParseTree(SqlAstNode* tree);

SQLDBLIB_EXPORT void SqlParser_PrintParseTree(SqlAstNode* tree);
SQLDBLIB_EXPORT void SqlParser_DumpTree(SqlAstNode* tree, const char *sql_raw, size_t sql_raw_len);