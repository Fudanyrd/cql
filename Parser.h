#pragma once

#include <iostream>

#include "expr_util.h"
#include "Partitioner.h"
#include "string_util.h"

namespace cql {

enum ExecutionType {
  Select,
  Update,
  Delete,
  Insert,
  Invalid
};

enum OrderByType {
  ASC,   // ascending
  DESC   // descending
};

const std::vector<std::string> keywords = {
  "select",
  "from",      // optional
  "where",     // optional
  "order by",  // optional
  "limit",     // optional
  "offset"     // optional
};

// work log generated by the parser.
struct ParserLog {
  ParserLog() = default;
  /**
   * @brief print the log to output stream.
   * For debugging.
   */
  void printTo(std::ostream &os) const;

  ExecutionType exec_type_{ExecutionType::Invalid};
  std::string table_;                                 // which table to select from.
  std::string update_column_;                         // column to be updated.
  std::vector<AbstractExprRef> columns_;              // column exprs.
  AbstractExprRef where_{nullptr};                    // where predicate.
  size_t limit_{static_cast<size_t>(-1)};             // limit the size of output.
  size_t offset_{0};
  std::vector<AbstractExprRef> order_by_;
  std::vector<OrderByType> order_by_type_;
};

class Parser {
 public:
 /**
  * @return the parser log given the parser.
  * the parser log can be used by planner to generate
  * execution plan.
  */
  static auto Parse(const Command &cmd) -> ParserLog;
};

}  // namespace cql
