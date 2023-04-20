#include <SqlDbLib/Database.h>
#include <SqlDbLib/Table.h>
#include <SqlDbLib/Record.h>

#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <list>
#include <unordered_set>
#include <unordered_map>


struct SqlDatabase
{
	std::string name;
	SqlDatabase_Error last_error {SQLDATABASE_ERROR_NONE};
	SqlDatabase_Result last_result {SQLDATABASE_RESULT_NONE};

	std::string error_buffer;
	void *last_result_data {nullptr};

	std::string cached_string;

	std::list<SqlTable *> tables;
	std::unordered_set<std::string> table_names;

	std::unordered_map<std::string, SqlTable *> cached_tables;
};

SqlDatabase *SqlDatabase_Create(const char *name)
{
	SqlDatabase *db = new SqlDatabase();
	db->name = name;
	return db;
}

void SqlDatabase_Destroy(SqlDatabase *db)
{
	for (auto &cached_table : db->cached_tables)
		SqlTable_Destroy(cached_table.second);

	delete db;
}

static void SqlDatabase_SetLastError(SqlDatabase *db, SqlDatabase_Error error, std::string &&string)
{
	db->last_error = error;
	db->error_buffer = std::move(string);
}

static void SqlDatabase_SetLastResult(SqlDatabase *db, SqlDatabase_Result result, void *data)
{
	db->last_result = result;
	db->last_result_data = data;
}

static void SqlDatabase_ClearState(SqlDatabase *db)
{
	db->last_error = SQLDATABASE_ERROR_NONE;
	db->last_result = SQLDATABASE_RESULT_NONE;
	db->error_buffer.clear();
}

//#if __BYTE_ORDER == __LITTLE_ENDIAN__
//#define SWAP_16(x) (((x) >> 8) | ((x) << 8))
//#define SWAP_32(x) (((x >> 24) & 0x000000FF) | ((x >> 8) & 0x0000FF00) | ((x << 8) & 0x00FF0000) | ((x << 24) & 0xFF000000))
//#else
//#define SWAP_16(x) (x)
//#define SWAP_32(x) (x)
//#endif
//
//#define WRITE_32(x, b) do { union { char v_8[4]; uint32_t v_32; } u{}; u.v_32 = SWAP_32(x); b->push_back(u.v_8[0]); b->push_back(u.v_8[1]); b->push_back(u.v_8[2]); b->push_back(u.v_8[3]); } while (0)
//#define WRITE_16(x, b) do { union { char v_8[2]; uuint32_t16_t v_16; } u{}; u.v_16 = SWAP_16(x); b->push_back(u.v_8[0]); b->push_back(u.v_8[1]); } while (0)
//#define WRITE_8(x, b) do { b->push_back(x); } while (0)
//
//#define READ_32(x, b) [](){ union { char v_8[4]; uint32_t v_32; } u{}; u.v_8[0] = b[0]; u.v_8[1] = b[1]; u.v_8[2] = b[2]; u.v_8[3] = b[3]; return SWAP_32(u.v_32); }()
//#define READ_16(x, b) [](){ union { char v_8[2]; uint16_t v_16; } u{}; u.v_8[0] = b[0]; u.v_8[1] = b[1]; return SWAP_16(u.v_16); }()
//#define READ_8(x, b) [](){ return b[0]; }()
//
//static bool SqlDatabase_Serialize(SqlDatabase *db, std::vector<char> *buffer)
//{
//	WRITE_32(db->name.size(), buffer);
//	for (auto name_it = db->name.begin(); name_it != db->name.end(); ++name_it)
//		WRITE_8(*name_it, buffer);
//	WRITE_32(db->tables.size(), buffer);
//	for (auto table_it = db->tables.begin(); table_it != db->tables.end(); table_it++)
//	{
//		const SqlDatabase_Table &table = *table_it;
//		WRITE_32(table.name.size(), buffer);
//		for (auto table_name_it = table.name.begin(); table_name_it != table.name.end(); ++table_name_it)
//		{
//			WRITE_8(*table_name_it, buffer);
//		}
//		WRITE_32(table.columns.size(), buffer);
//		for (auto column_it = table.columns.begin(); column_it != table.columns.end(); ++column_it)
//		{
//			WRITE_32(column_it->size(), buffer);
//			for (char c : *column_it)
//			{
//				WRITE_8(c, buffer);
//			}
//		}
//		WRITE_32(table.types.size(), buffer);
//		for (auto type_it = table.types.begin(); type_it != table.types.end(); ++type_it)
//		{
//			WRITE_32(type_it->size(), buffer);
//			for (char c : *type_it)
//			{
//				WRITE_8(c, buffer);
//			}
//		}
//
//		WRITE_32(table.records.size(), buffer);
//		for (auto record_it = table.records.begin(); record_it != table.records.end(); ++record_it)
//		{
//			WRITE_32(record_it->size(), buffer);
//			for (auto it2 = record_it->begin(); it2 != record_it->end(); ++it2)
//			{
//				WRITE_32(it2->size(), buffer);
//				for (char c : *it2)
//				{
//					WRITE_8(c, buffer);
//				}
//			}
//		}
//	}
//
//	return true;
//}
//
//static bool SqlDatabase_Deserialize(SqlDatabase *db, const std::vector<char> *buffer)
//{
//	return false;
//}

bool SqlDatabase_Load(SqlDatabase *db, const char *db_file)
{
	FILE *f = fopen(db_file, "rb");
	if (!f)
		return false;

	fseek(f, 0, SEEK_END);
	size_t file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	std::vector<char> buffer;
	buffer.resize(file_size);

	fread(buffer.data(), 1, file_size, f);
	fclose(f);

	return false;
//	return SqlDatabase_Deserialize(db, &buffer);
}

bool SqlDatabase_Save(SqlDatabase *db, const char *db_file)
{
	FILE *f = fopen(db_file, "wb");
	if (!f)
		return false;

	std::vector<char> buffer;
//	if (!SqlDatabase_Serialize(db, &buffer))
	{
		fclose(f);
		return false;
	}

	fwrite(buffer.data(), 1, buffer.size(), f);
	fclose(f);
	return false;
}

SqlDatabase_Error SqlDatabase_GetLastError(const SqlDatabase *db)
{
	return db->last_error;
}

SqlDatabase_Result SqlDatabase_GetLastResult(const SqlDatabase *db)
{
	return db->last_result;
}

void *SqlDatabase_GetLastResultData(const SqlDatabase *db)
{
	return db->last_result_data;
}

const char *SqlDatabase_GetLastErrorString(const SqlDatabase *db)
{
	return db->error_buffer.data();
}

bool SqlDatabase_CreateTable(SqlDatabase *db, const SqlDatabase_CreateTable_CreationData *creation_data)
{
	SqlDatabase_ClearState(db);

	if (db->table_names.count(creation_data->table_name))
	{
		std::string error_text = "Create table error: Table " + std::string(creation_data->table_name) + " already exists";
		SqlDatabase_SetLastError(db, SQLDATABASE_ERROR_TABLE_ALREADY_EXISTS, std::move(error_text));
		return false;
	}

	std::vector<std::string> query_columns;
	std::vector<std::string> query_types;

	{
		std::string columns = creation_data->columns;

		char *token = strtok(columns.data(), ",");
		while (token != nullptr)
		{
			query_columns.emplace_back(token);
			token = strtok(nullptr, ",");
		}
	}

	{
		std::string types = creation_data->types;

		const char *token = strtok(types.data(), ",");
		while (token != nullptr)
		{
			query_types.emplace_back(token);
			token = strtok(nullptr, ",");
		}

		if (query_types.size() != query_columns.size())
		{
			std::string error_text = "Create table error: Invalid type count. Expected " + std::to_string(query_columns.size()) + " but got " + std::to_string(query_types.size());
			SqlDatabase_SetLastError(db, SQLDATABASE_ERROR_INVALID_TYPE_COUNT, std::move(error_text));
			return false;
		}
	}

	// TODO: Refactor this
	std::vector<const char *> query_columns_cstr;
	std::vector<const char *> query_types_cstr;

	query_columns_cstr.reserve(query_columns.size());
	query_types_cstr.reserve(query_types.size());

	for (size_t i = 0; i < query_columns.size(); ++i)
	{
		query_columns_cstr.push_back(query_columns[i].c_str());
		query_types_cstr.push_back(query_types[i].c_str());
	}

	SqlTable *table = SqlTable_Create(creation_data->table_name, query_columns_cstr.data(), query_types_cstr.data(), query_columns.size());

	db->table_names.insert(creation_data->table_name);
	db->tables.emplace_back(table);

	return true;
}

bool SqlDatabase_DropTable(SqlDatabase *db, const char *table_name)
{
	SqlDatabase_ClearState(db);

	if (!db->table_names.count(table_name))
	{
		std::string error_text = "Drop table error: Table " + std::string(table_name) + " does not exist";
		SqlDatabase_SetLastError(db, SQLDATABASE_ERROR_TABLE_DOES_NOT_EXIST, std::move(error_text));
		return false;
	}

	db->tables.remove_if([table_name](const SqlTable *table) {
		return table_name == SqlTable_GetName(table);
	});
	db->table_names.erase(table_name);

	return true;
}

bool SqlDatabase_SelectQuery(SqlDatabase *db, const SqlDatabase_SelectQueryData *query_data)
{
	SqlDatabase_ClearState(db);

	std::string query_table_name = query_data->from_table;
	std::vector<std::string> query_columns;
	std::vector<std::string> query_types;

	if (!db->table_names.count(query_table_name))
	{
		std::string error_text = "Select query error: Table " + std::string(query_table_name) + " does not exist";
		SqlDatabase_SetLastError(db, SQLDATABASE_ERROR_TABLE_DOES_NOT_EXIST, std::move(error_text));
		return false;
	}

	SqlTable *source_table = *std::find_if(db->tables.begin(), db->tables.end(), [query_table_name](const SqlTable *table) {
		return query_table_name == SqlTable_GetName(table);
	});

	{
		std::string columns = query_data->columns;
		const char *column_token = strtok(columns.data(), ",");
		while (column_token != nullptr)
		{
			query_columns.emplace_back(column_token);
			query_types.emplace_back(SqlTable_GetColumnType(source_table, SqlTable_GetColumnIndex(source_table, column_token)));
			column_token = strtok(nullptr, ",");
		}
	}

	{
		bool error = false;
		for (const std::string &query_column: query_columns)
		{
			if (!SqlTable_HasColumn(source_table, query_column.c_str()))
			{
				std::string error_text = "Select query error: Column " + query_column + " does not exist";
				SqlDatabase_SetLastError(db, SQLDATABASE_ERROR_COLUMN_DOES_NOT_EXIST, std::move(error_text));
				error = true;
			}
		}
		if (error)
			return false;
	}

	// TODO: Refactor this
	std::vector<const char *> query_columns_cstr;
	std::vector<const char *> query_types_cstr;

	query_columns_cstr.reserve(query_columns.size());
	query_types_cstr.reserve(query_columns.size());

	for (size_t i = 0; i < query_columns.size(); ++i)
	{
		query_columns_cstr.push_back(query_columns[i].c_str());
		query_types_cstr.push_back(query_types[i].c_str());
	}

	SqlTable *result_table = SqlTable_Create("result", query_columns_cstr.data(), query_types_cstr.data(), query_columns.size());

	for (uint32_t i = 0; i < SqlTable_GetRecordCount(source_table); i++)
	{
		const SqlTableRecord *record = SqlTable_GetRecord(source_table, i);

		if (!SqlTableRecord_IsMatchingCondition(record, query_data->where, query_data->num_where))
			continue;

		std::vector<std::string> new_record;
		std::vector<const char *> new_record_cstr;
		new_record.reserve(query_columns.size());
		new_record_cstr.reserve(query_columns.size());
		for (const std::string &column : query_columns)
		{
			uint32_t column_index = SqlTable_GetColumnIndex(source_table, column.c_str());
			new_record.emplace_back(SqlTableRecord_GetValue(record, column_index));
			new_record_cstr.push_back(new_record.back().c_str());
		}

		SqlTable_AddRecord(result_table, new_record_cstr.data(), new_record.size());
	}

	db->cached_tables.insert({"", result_table});
	SqlDatabase_SetLastResult(db, SQLDATABASE_RESULT_TABLE, result_table);

	return true;
}

bool SqlDatabase_InsertQuery(SqlDatabase *db, const SqlDatabase_InsertQueryData *query_data)
{
	SqlDatabase_ClearState(db);

	if (!db->table_names.count(query_data->table_name))
	{
		std::string error_text = "Insertion error: Table " + std::string(query_data->table_name) + " does not exist";
		SqlDatabase_SetLastError(db, SQLDATABASE_ERROR_TABLE_DOES_NOT_EXIST, std::move(error_text));
		return false;
	}

	SqlTable *table = *std::find_if(db->tables.begin(), db->tables.end(), [query_data](const SqlTable *table) {
		return std::string(query_data->table_name) == SqlTable_GetName(table);
	});

	std::vector<std::string> query_columns;
	std::vector<std::vector<std::string>> query_bulk_values;

	{
		std::string columns = query_data->columns;
		const char *column_token = strtok(columns.data(), ",");
		while (column_token != nullptr)
		{
			query_columns.emplace_back(column_token);
			column_token = strtok(nullptr, ",");
		}
	}

	{
		bool error = false;
		query_bulk_values.reserve(query_data->num_bulk_values);
		for (uint32_t i = 0; i < query_data->num_bulk_values; ++i)
		{
			std::vector<std::string> v;
			v.reserve(query_columns.size());

			std::string csv_values = query_data->bulk_values[i];
			const char *value_token = strtok(csv_values.data(), ",");
			while (value_token != nullptr)
			{
				v.emplace_back(value_token);
				value_token = strtok(nullptr, ",");
			}

			if (v.size() != query_columns.size())
			{
				std::string error_text = "Insertion error: Invalid value count, doesn't match to count of inserting columns. Expected " + std::to_string(query_columns.size()) + " but got " + std::to_string(v.size());
				SqlDatabase_SetLastError(db, SQLDATABASE_ERROR_INVALID_VALUE_COUNT, std::move(error_text));
				error = true;
			}

			query_bulk_values.emplace_back(std::move(v));
		}
		if (error)
			return false;
	}

	{
		bool error = false;
		for (const std::string &query_column: query_columns)
		{
			if (!SqlTable_HasColumn(table, query_column.c_str()))
			{
				std::string error_text = "Insertion error: Column " + query_column + " does not exist";
				SqlDatabase_SetLastError(db, SQLDATABASE_ERROR_COLUMN_DOES_NOT_EXIST, std::move(error_text));
				error = true;
			}
		}
		if (error)
			return false;
	}

	for (uint32_t i = 0; i < query_bulk_values.size(); i++)
	{
		std::vector<std::string> new_record;
		std::vector<const char *> new_record_cstr;

		new_record.resize(query_columns.size());
		new_record_cstr.resize(query_columns.size());

		for (uint32_t j = 0; j < query_columns.size(); ++j)
		{
			uint32_t column_index = SqlTable_GetColumnIndex(table, query_columns[j].c_str());
			new_record[column_index] = query_bulk_values[i][j];
			new_record_cstr[column_index] = new_record[column_index].c_str();
		}

		SqlTable_AddRecord(table, new_record_cstr.data(), new_record_cstr.size());
	}

	return true;
}
