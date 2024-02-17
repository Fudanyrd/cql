#pragma once
#include <iostream>
#include <utility>
#include <string>
#include <vector>

#include "string_util.h"
#include "type.h"

namespace cql {

// for managing schema of a table.
class Schema {
 private:
  std::vector<std::pair<TypeId, std::string>> columns_;   // columns

  /**
   * Print a pair to output stream.
   */
  void printColumnTo(std::ostream &os, const std::pair<TypeId, std::string> &pair) const;

 public:
  Schema() = default;
  Schema(const Schema &that) { columns_ = that.columns_; }
  ~Schema() = default;
  
  /**
   * @brief initialize a schema using a header.
   * a header looks like:
   * col1:float, col2:char, ...
   */
  Schema(const std::string &header);

  /**
   * @brief Append a column to columns_
   * This is useful when constructing schemas.
   */
  void AppendCol(TypeId type, const std::string &name) { columns_.push_back({type, name}); }

  /**
   * @return the information of all columns.
   */
  auto getColumns() const -> const std::vector<std::pair<TypeId, std::string>> & { return columns_; }
  /**
   * @param idx: index in the vector.
   * @return one of the column.
   */
  auto getColumn(size_t idx) const -> const std::pair<TypeId, std::string> & { return columns_[idx]; }

  /**
   * @return number of columns.
   */
  auto getNumCols(void) const -> size_t { return columns_.size(); }

  /**
   * @brief print the schema to the given output stream.
   */
  void printTo(std::ostream &os) const;
};

}  // namespace cql