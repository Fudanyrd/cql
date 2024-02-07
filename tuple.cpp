#include <stdexcept>
#include "tuple.h"

namespace cql {

void Tuple::load(const std::string &line) {
  data_.clear();
  size_t i = 0;
  if (!static_cast<bool>(schema_)) {
    throw std::domain_error("loading an uninitialized tuple");
  }

  for (const auto &value : Split(line, ',')) {
    data_.push_back(DataBox(schema_->getColumn(i).first, value));
    ++i;
  }
}

void Tuple::dump(std::ostream &os) const {
  size_t i = 0;
  if (!static_cast<bool>(schema_)) {
    throw std::domain_error("dumping an uninitialized tuple");
  }
  // don't dump an deleted tuple!
  if (is_deleted_) { return; }

  data_[i++].printTo(os);
  for (; i < data_.size(); ++i) {
    os << ',';
    data_[i].printTo(os);
  }

  os << std::endl;
}

}  // namespace cql