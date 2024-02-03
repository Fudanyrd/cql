#pragma once
#include <iostream>
#include <vector>

#include "schema.h"
#include "string_util.h"
#include "type.h"

namespace cql {

// a tuple is a row of a table.
class Tuple {
 private:
  std::vector<DataBox> data_;       // data belong to the tuple.
  const Schema *schema_{nullptr};   // schema of the tuple.
  bool is_deleted_{false};          // mark if a tuple is deleted.

 public:
  Tuple() = default;
  Tuple(const Schema *schema): schema_(schema) {}

  /**
   * @brief load data from string(displaying a line from .csv files)
   */
  void load(const std::string &line);

  /**
   * @brief dump all data to the output stream.
   */
  void dump(std::ostream &os) const;

  /**
   * @return the schema of the tuple.
   */
  auto getSchema() const -> const Schema * { return schema_; }

  /**
   * @return the data of all columns.
   */
   auto getData() const -> const std::vector<DataBox> & { return data_; }

  /**
   * @return the data at a given column.
   */
  auto getColumnData(size_t column_idx) const -> DataBox { return data_[column_idx]; }

   /**
    * @brief logically delete a tuple.
    * @return true if successful.
    */
   auto markDelete() -> bool {
     if (!is_deleted_) {
      is_deleted_ = true;
      return true;
     }
     return false;
   }
};

}  // namespace cql