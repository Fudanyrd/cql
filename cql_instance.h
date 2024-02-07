/************************************************
 * cql_instance.h
 * 
 * Actually take in queries, and fetch data from
 * cache, and execute query and get result.
 ************************************************/
#pragma once
#include <fstream>
#include <unordered_map>

#include "expr_util.h"
#include "expr.h"
#include "Partitioner.h"
#include "Parser.h"
#include "string_util.h"
#include "table.h"
#include "variable_manager.h"

namespace cql {

struct TableInfo {
  TableInfo() = default;
  TableInfo(Table *ptr, bool dirty = false): table_ptr_(ptr), is_dirty_(dirty) {}
  // if dirty, dump the table to where it should be.
  ~TableInfo() = default;

  Table *table_ptr_{nullptr};             // table pointer.
  bool is_dirty_{false};                  // whether table is dirty.
};

// it's time to run cql Kenel! how exciting.
class cqlInstance {
 private:
  VariableManager var_mgn_;                               // manage the variable defined by users.
  std::unordered_map<std::string, TableInfo> table_mgn_;  // table manager.

  void execute(const Command &complete);
 public:
  cqlInstance() = default;
  // disallow copy.
  cqlInstance(const cqlInstance &that) = delete;
  // it has to flush all dirty tables into disk.
  ~cqlInstance();

  /**
   * @brief run the cql kenel.
   */
  void run();

  /**
   * Flush dirty table to disk.
   */
  void dump();
};

}  // namespace cql
