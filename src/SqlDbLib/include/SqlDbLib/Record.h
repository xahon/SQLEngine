#pragma once

#include <SqlDbLib/Export.h>

#include <cstdint>
#include <cstdbool>

typedef struct SqlTable SqlTable;
typedef struct SqlTableRecord SqlTableRecord;

SQLDBLIB_EXPORT SqlTableRecord *SqlTableRecord_Create(const SqlTable *table, const char **values, size_t num_values);
SQLDBLIB_EXPORT void SqlTableRecord_Destroy(SqlTableRecord *record);

SQLDBLIB_EXPORT bool SqlTableRecord_IsMatchingCondition(const SqlTableRecord *record, const char **where, size_t num_where);

SQLDBLIB_EXPORT size_t SqlTableRecord_GetColumnCount(const SqlTableRecord *record);
SQLDBLIB_EXPORT const char *SqlTableRecord_GetColumnName(const SqlTableRecord *record, size_t column_index);
SQLDBLIB_EXPORT const char *SqlTableRecord_GetColumnType(const SqlTableRecord *record, size_t column_index);
SQLDBLIB_EXPORT const char *SqlTableRecord_GetValue(const SqlTableRecord *record, size_t value_index);