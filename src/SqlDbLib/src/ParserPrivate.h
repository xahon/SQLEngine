#pragma once

enum SqlTokenType
{
	SQL_TOKEN_TYPE_UNKNOWN = 0,
	SQL_TOKEN_TYPE_KEYWORD,
	SQL_TOKEN_TYPE_IDENTIFIER,
	SQL_TOKEN_TYPE_STRING,
	SQL_TOKEN_TYPE_NUMBER,
	SQL_TOKEN_TYPE_OPERATOR,
	SQL_TOKEN_TYPE_BOOLEAN,
};

enum SqlKeywordType
{
	SQL_KEYWORD_TYPE_UNKNOWN = 0,
	SQL_KEYWORD_TYPE_AND,
	SQL_KEYWORD_TYPE_OR,
	SQL_KEYWORD_TYPE_NOT,
	SQL_KEYWORD_TYPE_TRUE,
	SQL_KEYWORD_TYPE_FALSE,
	SQL_KEYWORD_TYPE_WHERE,
	SQL_KEYWORD_TYPE_ORDER,
	SQL_KEYWORD_TYPE_BY,
	SQL_KEYWORD_TYPE_SELECT,
	SQL_KEYWORD_TYPE_FROM,
	SQL_KEYWORD_TYPE_INSERT,
	SQL_KEYWORD_TYPE_INTO,
	SQL_KEYWORD_TYPE_VALUES,
	SQL_KEYWORD_TYPE_UPDATE,
	SQL_KEYWORD_TYPE_SET,
	SQL_KEYWORD_TYPE_DELETE,
};

struct SqlToken {
	const char *buffer;
	size_t size;
	SqlTokenType type;
	SqlKeywordType keyword_type;
};
