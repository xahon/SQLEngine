#pragma once

#include <SqlDbLib/Parser.h>
#include "ParserPrivate.h"

#include <deque>
#include <stack>

// #define AUTOGEN_PARSER_DEBUG

SqlAstNode *SqlParseTree_Parse_Root(std::deque<SqlToken *> &q);
SqlAstNode *SqlParseTree_Parse_Query(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Queries(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_QueryList(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_SelectQuery(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Select(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_From(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Where(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_InsertQuery(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_UpdateQuery(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_DeleteQuery(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Set(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_CommaSet(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Column(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Columns(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_CommaColumns(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_PeriodColumn(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Table(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Tables(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_CommaTables(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_CmpCompound(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Cmp(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Value(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Values(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_ValueListComma(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_String(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Identifier(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Number(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Boolean(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Keyword(std::deque<SqlToken *> &q, SqlAstNode *parent);
SqlAstNode *SqlParseTree_Parse_Operator(std::deque<SqlToken *> &q, SqlAstNode *parent);
