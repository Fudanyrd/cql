#pragma once
#include <vector>

#include "schema.h"
#include "tuple.h"

// we assume that a table can fit perfectly into the memory.
namespace cql {

// a in-memory table manager
class Table {
 private:
  Schema schema_;               // schema of the table.
  std::vector<Tuple> tuples_;   // tuples of the table.

 public:
  Table() = default;
  ~Table() = default;

  /**
   * @brief load from a .csv file.
   * @param filename: the file to read from.
   * @return number of tuples read from file.
   */
  auto load(const std::string &filename) -> size_t;

  /**
   * @brief dump to a .csv file.
   */
  void dump(std::ostream &os) const;

  /**
   * @return the schema of the table.
   */
  auto getSchema() const -> const Schema * { return &schema_; }

  /**
   * @return the tuples of the table.
   */
  auto getTuples() const -> const std::vector<Tuple> & { return tuples_; }

  /**
   * @return number of rows in the table.
   */
  auto getNumRows() const -> size_t { return tuples_.size(); }
};

}