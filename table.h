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
  Table(const std::string &header): schema_({header}) {}
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

  /**
   * @brief insert a tuple into the table.
   */
  void insertTuple(const std::vector<DataBox> &data) { tuples_.push_back({&schema_, data}); }

  /**
   * @brief delete the tuple on index.
   */
  auto deleteTuple(size_t idx) -> bool { return tuples_[idx].markDelete(); }

  /**
   * @brief update the column of a tuple.
   */
  auto updateTuple(const DataBox &box, size_t row, size_t col) -> bool {
    return tuples_[row].update(box, col);
  }
};

/** Useful collection of a table information */
struct TableInfo {
  TableInfo() = default;
  TableInfo(Table *ptr, bool dirty = false): table_ptr_(ptr), is_dirty_(dirty) {}
  // if dirty, dump the table to where it should be(but not destroy the table explicitly).
  ~TableInfo() = default;

  Table *table_ptr_{nullptr};             // table pointer.
  bool is_dirty_{false};                  // whether table is dirty.
};

}