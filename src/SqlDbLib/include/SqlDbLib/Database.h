#pragma once

#include <SqlDbLib/Export.h>

#include <cstdint>
#include <cstdbool>

typedef struct SqlDatabase SqlDatabase;
typedef struct SqlTable SqlTable;

typedef struct SqlDatabase_CreateTable_CreationData
{
	const char *table_name;
	const char *columns;
	const char *types;
} SqlDatabase_CreateTable_CreationData;

typedef struct SqlDatabase_SelectQueryData
{
	const char *from_table;
	const char *columns;
	const char **where;
	size_t num_where;
} SqlDatabase_SelectQueryData;

typedef struct SqlDatabase_InsertQueryData
{
	const char *table_name;
	const char *columns;
	const char **bulk_values;
	size_t num_bulk_values;
} SqlDatabase_InsertQueryData;

typedef enum SqlDatabase_Error
{
	SQLDATABASE_ERROR_NONE = 0,
	SQLDATABASE_ERROR_TABLE_ALREADY_EXISTS,
	SQLDATABASE_ERROR_TABLE_DOES_NOT_EXIST,
	SQLDATABASE_ERROR_INVALID_COLUMN_COUNT,
	SQLDATABASE_ERROR_INVALID_TYPE_COUNT,
	SQLDATABASE_ERROR_COLUMN_DOES_NOT_EXIST,
	SQLDATABASE_ERROR_INVALID_VALUE_COUNT,
} SqlDatabase_Error;

typedef enum SqlDatabase_Result
{
	SQLDATABASE_RESULT_NONE = 0,
	SQLDATABASE_RESULT_TABLE,
} SqlDatabase_Result;

SQLDBLIB_EXPORT SqlDatabase *SqlDatabase_Create(const char *name);
SQLDBLIB_EXPORT void SqlDatabase_Destroy(SqlDatabase *db);

SQLDBLIB_EXPORT bool SqlDatabase_Load(SqlDatabase *db, const char *db_file);
SQLDBLIB_EXPORT bool SqlDatabase_Save(SqlDatabase *db, const char *db_file);

SQLDBLIB_EXPORT SqlDatabase_Error SqlDatabase_GetLastError(const SqlDatabase *db);
SQLDBLIB_EXPORT SqlDatabase_Result SqlDatabase_GetLastResult(const SqlDatabase *db);

SQLDBLIB_EXPORT void *SqlDatabase_GetLastResultData(const SqlDatabase *db);
SQLDBLIB_EXPORT const char *SqlDatabase_GetLastErrorString(const SqlDatabase *db);

SQLDBLIB_EXPORT bool SqlDatabase_CreateTable(SqlDatabase *db, const SqlDatabase_CreateTable_CreationData *creation_data);
SQLDBLIB_EXPORT bool SqlDatabase_DropTable(SqlDatabase *db, const char *table_name);
SQLDBLIB_EXPORT bool SqlDatabase_SelectQuery(SqlDatabase *db, const SqlDatabase_SelectQueryData *query_data);
SQLDBLIB_EXPORT bool SqlDatabase_InsertQuery(SqlDatabase *db, const SqlDatabase_InsertQueryData *query_data);
