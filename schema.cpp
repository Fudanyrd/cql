#include "schema.h"

namespace cql {

void Schema::printColumnTo(std::ostream &os, const std::pair<TypeId, std::string> &pair) const {
  // output the column name.
  os << pair.second;
  switch (pair.first) {
    case TypeId::Float:
      os << ":float"; break;
    case TypeId::Char:
      os << ":char";  break;
    default:
      os << ":invalid";  break;
  }
}

void Schema::printTo(std::ostream &os) const {
  size_t i = 0;
  this->printColumnTo(os, columns_[i]);

  // print the rest of columns.
  for(++i; i < columns_.size(); ++i) {
    os << ',';
    this->printColumnTo(os, columns_[i]);
  }

  os << std::endl;
}

Schema::Schema(const std::string &header) {
  // split the header by commas.
  auto columns = Split(header, ',');
  for (const auto &str : columns) {
    // get the current column.
    std::vector<std::string> column = Split(str, ':');
    bool matched = false;
    if (column[1] == "float") {
      matched = true;
      columns_.push_back(std::pair<TypeId, std::string>(TypeId::Float, column[0]));
    }
    if (column[1] == "char") {
      matched = true;
      columns_.push_back(std::pair<TypeId, std::string>(TypeId::Char, column[0]));
    }
    if (!matched) {
      columns_.push_back(std::pair<TypeId, std::string>(TypeId::INVALID, column[0]));
    }
    
  }
}

}  // namespace cql