#pragma once
#include "dbengine.h"
#include <sqlite3.h>
#include <tuple>
#include <iostream>

constexpr auto kTempTableSubFix {"_TEMP"};

enum ColumnType {
  UNKNOWN_TYPE = 0,
  TEXT_TYPE,
  INTEGER_TYPE,
  BIGINT_TYPE,
  UNSIGNED_BIGINT_TYPE,
  DOUBLE_TYPE,
  BLOB_TYPE,
};

const std::map<ColumnType, std::string> kColumnTypeNames = {
    {UNKNOWN_TYPE, "UNKNOWN"},
    {TEXT_TYPE, "TEXT"},
    {INTEGER_TYPE, "INTEGER"},
    {BIGINT_TYPE, "BIGINT"},
    {UNSIGNED_BIGINT_TYPE, "UNSIGNED BIGINT"},
    {DOUBLE_TYPE, "DOUBLE"},
    {BLOB_TYPE, "BLOB"},
};

enum TableHeader {
  CID = 0,
  NAME,
  TYPE,
  PK
}; 
using ColumnData = 
    std::tuple<int32_t, std::string, ColumnType, bool>;

using TableColumns =
    std::vector<ColumnData>;

enum GenericTupleIndex {
  GEN_TYPE = 0,
  GEN_STRING,
  GEN_INTEGER,
  GEN_BIGINT,
  GEN_UNSIGNED_BIGINT,
  GEN_DOUBLE
}; 

using TableField = 
    std::tuple<int32_t, std::string, int32_t, int64_t, uint64_t, double_t>;

using Row = std::map<std::string, TableField>;


enum ResponseType {
  RT_JSON = 0,
  RT_CALLBACK
}; 

class SQLiteDBEngine : public DbEngine {
public:
  SQLiteDBEngine(const std::string& path, const std::string& table_statement_creation);
  ~SQLiteDBEngine();
  
  virtual bool Execute(const std::string& query) override;
  virtual bool Select(const std::string& query, nlohmann::json& result) override;
  virtual bool BulkInsert(const std::string& table, const nlohmann::json& data) override;
  virtual bool RefreshTablaData(const nlohmann::json& data, const std::tuple<nlohmann::json&, void *> delta) override;

private:
  bool Initialize(const std::string& path, const std::string& table_statement_creation);
  bool CleanDB(const std::string& path);
  
  size_t LoadTableData(const std::string& table);
  bool LoadFieldData(const std::string& table);
  std::string BuildInsertBulkDataSqlQuery(const std::string& table);
  std::string BuildDeleteBulkDataSqlQuery(const std::string& table, const std::vector<std::string>& primary_key_list);
  ColumnType ColumnTypeName(const std::string& type);
  int32_t BindJsonData(sqlite3_stmt* stmt, const ColumnData& cd, const nlohmann::json::value_type& value_type);

  bool CreateCopyTempTable(const std::string& table);
  bool GetTableCreateQuery(const std::string& table, std::string& result_query);
  bool GetPrimaryKeysFromTable(const std::string& table, std::vector<std::string>& primary_key_list);

  bool RemoveNotExistsRows(const std::string& table, const std::vector<std::string>& primary_key_list, const std::tuple<nlohmann::json&, void *> delta);
  bool InsertNewRows(const std::string& table, const std::vector<std::string>& primary_key_list, const std::tuple<nlohmann::json&, void *> delta);

  bool DeleteRows(const std::string& table, const std::vector<std::string>& primary_key_list, const std::vector<Row>& rows_to_remove); 
  int32_t GetTableData(sqlite3_stmt* stmt, const int32_t index, const ColumnType& type, const std::string& field_name, Row& row);
  int32_t BindFieldData(sqlite3_stmt* stmt, const int32_t index, const TableField& field_data);

  std::string BuildLeftOnlyQuery(const std::string& t1,const std::string& t2,const std::vector<std::string>& primary_key_list, const bool return_only_pk_fields = false);
  bool GetLeftOnly(const std::string& t1,const std::string& t2, const std::vector<std::string>& primary_key_list, std::vector<Row>& return_rows);
  bool GetPKListLeftOnly(const std::string& t1, const std::string& t2, const std::vector<std::string>& primary_key_list, std::vector<Row>& return_rows);
  bool BulkInsert(const std::string& table, const std::vector<Row>& data);
  int DeleteTempTable(const std::string& table);

  std::string BuildModifiedRowsQuery(const std::string& t1,const std::string& t2, const std::vector<std::string>& primary_key_list);
  int ChangeModifiedRows(const std::string& table, const std::vector<std::string>& primary_key_list, const std::tuple<nlohmann::json&, void *> delta);
  std::string BuildUpdateDataSqlQuery(const std::string& table, const std::vector<std::string>& primary_key_list, const Row& row, const std::pair<const std::__cxx11::string, TableField> &field);

  int32_t PreparedTransactionExecute(const std::string& sql, const std::function<int32_t(sqlite3_stmt*)>& bind_f);
  int32_t TransactionExecute(const std::function<int32_t()>& bind_f);

  bool GetFieldValueFromTuple(const std::pair<const std::__cxx11::string, TableField> &value, std::string& result_value, const bool quotation_marks = false);
  bool GetFieldValueFromTuple(const std::pair<const std::__cxx11::string, TableField> &value, nlohmann::json& object);

  SQLiteDBEngine(const SQLiteDBEngine&) = delete;
  SQLiteDBEngine& operator=(const SQLiteDBEngine&) = delete;

  std::map<std::string, TableColumns> m_table_fields;
  sqlite3* m_db;

};