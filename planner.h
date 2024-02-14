/**********************************************************
 * planner.h
 * Author: Fudanyrd (email: yangrundong7@gmail.com)
 *
 * Part of the Parser.
 * Given the parser log, generate the execution tree.
 **********************************************************/
#pragma once

#include "executor.h"
#include "Parser.h"
#include "variable_manager.h"

namespace cql {

class Planner {
 private:
  /** Table manager to use */
  std::unordered_map<std::string, TableInfo> *table_mgn_{nullptr};
  /** Variable manager to use */
  VariableManager *var_mgn_{nullptr};

 public:
  Planner() = default;
  Planner(std::unordered_map<std::string, TableInfo> *tb_mgn, VariableManager *var_mgn):
    table_mgn_(tb_mgn), var_mgn_(var_mgn) {}
  ~Planner() = default;

  auto GetExecutors(const ParserLog &log) -> AbstractExecutorRef;
};

}  // namespace cql