#pragma once

#include <SqlDbLib/Export.h>

#include <cstdint>
#include <cstdbool>

typedef struct SqlDatabase SqlDatabase;
typedef struct SqlTable SqlTable;
typedef struct SqlTableRecord SqlTableRecord;

SQLDBLIB_EXPORT SqlTable *SqlTable_Create(const char *table_name, const char **columns, const char **types, size_t num_columns);
SQLDBLIB_EXPORT void SqlTable_Destroy(SqlTable *table);

SQLDBLIB_EXPORT SqlTableRecord *SqlTable_AddRecord(SqlTable *table, const char **values, size_t num_values);

SQLDBLIB_EXPORT const char *SqlTable_GetName(const SqlTable *table);
SQLDBLIB_EXPORT size_t SqlTable_GetVersion(const SqlTable *table);
SQLDBLIB_EXPORT size_t SqlTable_GetRecordCount(const SqlTable *table);
SQLDBLIB_EXPORT const SqlTableRecord *SqlTable_GetRecord(const SqlTable *table, size_t record_index);
SQLDBLIB_EXPORT bool SqlTable_HasColumn(const SqlTable *table, const char *column_name);

SQLDBLIB_EXPORT size_t SqlTable_GetColumnCount(const SqlTable *table);
SQLDBLIB_EXPORT const char *SqlTable_GetColumnName(const SqlTable *table, size_t column_index);
SQLDBLIB_EXPORT size_t SqlTable_GetColumnIndex(const SqlTable *table, const char *name);
SQLDBLIB_EXPORT const char *SqlTable_GetColumnType(const SqlTable *table, size_t column_index);

SQLDBLIB_EXPORT const char *SqlTable_ToString(SqlDatabase *db, const SqlTable *table);