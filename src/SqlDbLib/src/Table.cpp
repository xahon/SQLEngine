#include <SqlDbLib/Table.h>
#include <SqlDbLib/Record.h>

#include <unordered_set>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>


struct SqlTable
{
	std::string name;
	SqlTableRecord *columns;
	SqlTableRecord *types;
	std::vector<SqlTableRecord *> records;

	size_t version {0};
	std::unordered_set<std::string> columns_set;
	std::unordered_map<std::string, uint32_t> column_name_to_index;

	mutable std::string to_string_cache;
};

SqlTable *SqlTable_Create(const char *table_name, const char **columns, const char **types, size_t num_columns)
{
	SqlTable *table = new SqlTable();
	table->name = table_name;
	table->columns = SqlTableRecord_Create(table, columns, num_columns);
	table->types = SqlTableRecord_Create(table, types, num_columns);
	table->version = 1;

	for (size_t i = 0; i < num_columns; i++)
	{
		table->columns_set.insert(SqlTableRecord_GetValue(table->columns, i));
		table->column_name_to_index[SqlTableRecord_GetValue(table->columns, i)] = i;
	}

	return table;
}

void SqlTable_Destroy(SqlTable *table)
{
	delete table;
}

SqlTableRecord *SqlTable_AddRecord(SqlTable *table, const char **values, size_t num_values)
{
	SqlTableRecord *record = SqlTableRecord_Create(table, values, num_values);

	table->records.push_back(record);
	table->to_string_cache.clear();
	table->version++;

	return record;
}

const char *SqlTable_GetName(const SqlTable *table)
{
	return table->name.c_str();
}

size_t SqlTable_GetVersion(const SqlTable *table)
{
	return table->version;
}

size_t SqlTable_GetColumnCount(const SqlTable *table)
{
	return SqlTableRecord_GetColumnCount(table->columns);
}

const char *SqlTable_GetColumnName(const SqlTable *table, size_t column_index)
{
	return SqlTableRecord_GetValue(table->columns, column_index);
}

size_t SqlTable_GetColumnIndex(const SqlTable *table, const char *name)
{
	return table->column_name_to_index.at(name);
}

const char *SqlTable_GetColumnType(const SqlTable *table, size_t column_index)
{
	return SqlTableRecord_GetValue(table->types, column_index);
}

size_t SqlTable_GetRecordCount(const SqlTable *table)
{
	return table->records.size();
}

const SqlTableRecord *SqlTable_GetRecord(const SqlTable *table, size_t record_index)
{
	return table->records.at(record_index);
}

bool SqlTable_HasColumn(const SqlTable *table, const char *column_name)
{
	return table->columns_set.count(column_name);
}

const char *SqlTable_ToString(SqlDatabase *db, const SqlTable *table)
{
	if (!table->to_string_cache.empty())
		return table->to_string_cache.c_str();

	std::vector<std::pair<size_t, size_t>> columns_offsets(SqlTable_GetColumnCount(table), { 0, 0 });

	uint32_t horizontal_pads = 1;
	size_t last_end = 0;

	const auto expand_offsets = [&columns_offsets](size_t start, size_t end, size_t i)
	{
		if (columns_offsets[i].first < start || columns_offsets[i].second < end)
		{
			size_t additional_offset = end - columns_offsets[i].second;

			columns_offsets[i].first = start;
			columns_offsets[i].second = end;
			for (uint32_t j = i + 1; j < columns_offsets.size(); j++)
			{
				columns_offsets[j].first += additional_offset;
				columns_offsets[j].second += additional_offset;
			}
		}
		return columns_offsets[i].second;
	};

	// Calculate max sizes for each column in header and records
	for (uint32_t i = 0; i < columns_offsets.size(); i++)
	{
		size_t start = last_end;
		size_t end = start + strlen(SqlTable_GetColumnName(table, i)) + horizontal_pads * 2 + 1;
		last_end = expand_offsets(start, end, i);
	}
	for (const SqlTableRecord *record: table->records)
	{
		last_end = 0;
		for (uint32_t i = 0; i < columns_offsets.size(); i++)
		{
			size_t start = last_end;
			size_t end = start + strlen(SqlTableRecord_GetValue(record, i)) + horizontal_pads * 2 + 1;
			last_end = expand_offsets(start, end, i);
		}
	}

	if (columns_offsets.empty())
	{
		return "<No columns>";
	}

	std::unordered_set<size_t> column_offsets_set;
	for (const std::pair<size_t, size_t> &offsets : columns_offsets)
	{
		column_offsets_set.insert(offsets.first);
		column_offsets_set.insert(offsets.second);
	}

	std::stringstream ss;

	// Build horizontal markings string
	size_t max_width = columns_offsets.back().second;
	std::string horizontal_markings_ss(columns_offsets.back().second + 1, '-');
	for (size_t i = 0; i <= max_width; i++)
		if (column_offsets_set.count(i))
			horizontal_markings_ss[i] = '+';

	const auto print_table_row = [&columns_offsets](const SqlTableRecord *row_values, std::stringstream &ss)
	{
		for (uint32_t i = 0; i < SqlTableRecord_GetColumnCount(row_values); ++i)
		{
			const std::string &column_name = SqlTableRecord_GetValue(row_values, i);
			size_t str_size = columns_offsets[i].second - columns_offsets[i].first;
			size_t padding = 2;

			if (i == 0)
			{
				str_size += 1;
				padding -= 1;
			}

			std::string column(str_size, ' ');

			if (i == 0)
				column[0] = '|';
			column[column.size() - 1] = '|';

			size_t start_index = columns_offsets[i].second - columns_offsets[i].first - column_name.size() - padding;

			for (uint32_t j = 0; j < column_name.size(); j++)
				column[start_index + j] = column_name[j];
			ss << column;
		}
	};

	// Print header lines
	ss << horizontal_markings_ss << "\n";
	print_table_row(table->columns, ss);
	ss << "\n" << horizontal_markings_ss << "\n";

	// Print records. In case of empty list, don't print horizontal markings
	if (!table->records.empty())
	{
		for (const SqlTableRecord *record : table->records)
		{
			print_table_row(record, ss);
			ss << "\n";
		}
		ss << horizontal_markings_ss << "\n";
	}

	table->to_string_cache = ss.str();
	return table->to_string_cache.c_str();
}
