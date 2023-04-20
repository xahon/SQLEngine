#include <SqlDbLib/Parser.h>
#include <SqlDbLib/Database.h>
#include <SqlDbLib/Table.h>

#include <cstdio>
#include <sstream>
#include <algorithm>
#include <filesystem>

int main()
{
	SqlDatabase *db = SqlDatabase_Create("test");

	SqlDatabase_CreateTable_CreationData creation_data;
	creation_data.table_name = "people";
	creation_data.columns = "id,name,age";
	creation_data.types = "int,string,int";
	if (!SqlDatabase_CreateTable(db, &creation_data))
	{
		printf("Error: %s", SqlDatabase_GetLastErrorString(db));
		return 1;
	}

	SqlDatabase_InsertQueryData insert_data;
	insert_data.table_name = "people";
	insert_data.columns = "id,name,age";
	const char *values[] = {
		"1,'John',20",
		"2,'Jane',21",
		"3,'Joe',22",
		"4,'Jack',23",
		"5,'Alice',24",
		"6,'Bob',25",
		"7,'Charlie',26",
		"8,'Dave',27",
		"9,'Eve',28",
	};
	insert_data.bulk_values = values;
	insert_data.num_bulk_values = sizeof(values) / sizeof(values[0]);
	if (!SqlDatabase_InsertQuery(db, &insert_data))
	{
		printf("Error: %s", SqlDatabase_GetLastErrorString(db));
		return 1;
	}

	SqlDatabase_SelectQueryData select_data;
	select_data.from_table = "people";
	select_data.columns = "id,age,name";
	const char *where[] = {
		"age >= 24",
	};
	select_data.where = where;
	select_data.num_where = sizeof(where) / sizeof(where[0]);
	if (!SqlDatabase_SelectQuery(db, &select_data))
	{
		printf("Error: %s", SqlDatabase_GetLastErrorString(db));
		return 1;
	}

	if (SqlDatabase_GetLastResult(db) == SQLDATABASE_RESULT_TABLE)
	{
		SqlTable *table = (SqlTable *)SqlDatabase_GetLastResultData(db);
		printf("%s\n", SqlTable_ToString(db, table));
	}
	else
	{
		printf("No results\n");
	}

//	SqlDatabase_Save(db, "test.db");

	SqlDatabase_Destroy(db);

//	const char *sql_query =
//"SELECT `aa` FROM `bb` WHERE `cc` = 20; "
//"INSERT INTO `table` (`column1`, `column2`) VALUES (1, 2); "
//"DELETE FROM `table` WHERE `column1` = 1; "
//"UPDATE `table` SET `column1` = 1, `column2` = 2 WHERE `column3` = 3; "
//;
//	size_t sql_query_len = strlen(sql_query);
//
//	SqlAstNode *tree = SqlParser_Parse(sql_query, sql_query_len);
////	SqlParser_PrintParseTree(tree);
//	SqlParser_DumpTree(tree, sql_query, sql_query_len);
//	SqlParse_DestroyParseTree(tree);

	return 0;
}