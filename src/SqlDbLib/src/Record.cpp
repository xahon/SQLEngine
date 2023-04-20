#include <SqlDbLib/Record.h>
#include <SqlDbLib/Table.h>

#include <vector>
#include <string>


struct SqlTableRecord
{
	std::vector<std::string> values;

	const SqlTable *table;
};

SqlTableRecord *SqlTableRecord_Create(const SqlTable *table, const char **values, size_t num_values)
{
	SqlTableRecord *record = new SqlTableRecord();
	record->table = table;
	record->values.reserve(num_values);

	for (size_t i = 0; i < num_values; i++)
		record->values.emplace_back(values[i]);

	return record;
}

void SqlTableRecord_Destroy(SqlTableRecord *record)
{
	delete record;
}


enum SqlTable_OperandType
{
	SqlTable_OperandType_Int,
	SqlTable_OperandType_String,
};

template <SqlTable_OperandType TType>
static bool SqlTable_EvalCondition(const char *op_left, const char *op, const char *op_right);

template<typename T>
static bool SqlTable_EvalConditionOps(const T &left, const T &right, const char *op)
{
	if (strcmp(op, "=") == 0)
		return left == right;
	else if (strcmp(op, ">") == 0)
		return left > right;
	else if (strcmp(op, ">=") == 0)
		return left >= right;
	else if (strcmp(op, "<") == 0)
		return left < right;
	else if (strcmp(op, "<=") == 0)
		return left <= right;
	else if (strcmp(op, "!=") == 0)
		return left != right;

	return false;
}

template <>
static bool SqlTable_EvalCondition<SqlTable_OperandType_Int>(const char *op_left, const char *op, const char *op_right)
{
	int32_t left = atoi(op_left);
	int32_t right = atoi(op_right);
	return SqlTable_EvalConditionOps(left, right, op);
}

template <>
static bool SqlTable_EvalCondition<SqlTable_OperandType_String>(const char *op_left, const char *op, const char *op_right)
{
	std::string left = op_left;
	std::string right = op_right;
	return SqlTable_EvalConditionOps(left, right, op);
}

bool SqlTableRecord_IsMatchingCondition(const SqlTableRecord *record, const char **where, size_t num_where)
{
	const SqlTable *table = record->table;

	for (size_t i = 0; i < num_where; i++)
	{
		std::string where_str = where[i];
		const char *op_left = strtok(where_str.data(), " ");
		const char *op = strtok(nullptr, " ");
		const char *op_right = strtok(nullptr, " ");

		std::string column;
		bool column_found = false;

		for (size_t j = 0; j < SqlTable_GetColumnCount(table); j++)
		{
			std::string column_name = SqlTable_GetColumnName(table, j);
			if (column_name == op_left)
			{
				column = column_name;
				column_found = true;
				break;
			}
		}

		if (!column_found)
			return false;

		size_t column_index = SqlTable_GetColumnIndex(table, column.c_str());
		std::string column_type = SqlTable_GetColumnType(table, column_index);

		op_left = SqlTableRecord_GetValue(record, column_index);

		if (column_type == "int")
		{
			if (!SqlTable_EvalCondition<SqlTable_OperandType_Int>(op_left, op, op_right))
				return false;
		}
		else if (column_type == "string")
		{
			if (!SqlTable_EvalCondition<SqlTable_OperandType_String>(op_left, op, op_right))
				return false;
		}
	}

	return true;
}

size_t SqlTableRecord_GetColumnCount(const SqlTableRecord *record)
{
	return record->values.size();
}

const char *SqlTableRecord_GetColumnName(const SqlTableRecord *record, size_t column_index)
{
	return SqlTable_GetColumnName(record->table, column_index);
}

const char *SqlTableRecord_GetColumnType(const SqlTableRecord *record, size_t column_index)
{
	return SqlTable_GetColumnType(record->table, column_index);
}

const char *SqlTableRecord_GetValue(const SqlTableRecord *record, size_t value_index)
{
	return record->values[value_index].c_str();
}
