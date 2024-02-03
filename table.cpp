#include <fstream>
#include <stdexcept>
#include "table.h"

namespace cql {

auto Table::load(const std::string &filename) -> size_t {
  // clear initial data.
  tuples_.clear();
  std::string header;
  std::fstream fin(filename.c_str());
  if (!fin.is_open()) {
    throw std::domain_error("cannot open file");
  }

  // get the header and initialize schema.
  if (!getline(fin, header)) {
    throw std::domain_error("cannot fetch header");
  }
  schema_ = Schema(header);

  // get the tuples.
  size_t count = 0;
  std::string row;
  while (getline(fin, row)) {
    tuples_.push_back(Tuple(&schema_));
    tuples_[count].load(row);
    ++count;
  }

  // return the number of tuples read from file.
  return count;
}

void Table::dump(std::ostream &os) const {
  schema_.printTo(os);
  for (const auto &tuple : tuples_) {
    tuple.dump(os);
  }
}

}   // namespace cql
