#include "type.h"

namespace cql {

DataBox::DataBox(TypeId tp, const std::string &dat) {
  this->type_ = tp;
  switch(tp) {
    case TypeId::INVALID:
      break;
    case TypeId::Char:
      this->str_dat_ = dat;
      break;
    case TypeId::Float:
      this->float_dat_ = atof(dat.c_str());
      break;
  }
}

void DataBox::printTo(std::ostream &os) const {
  switch(type_) {
    case TypeId::INVALID:
      os << "NULL";  // invalid... maybe just null?
      break;
    case TypeId::Char:
      os << str_dat_;
      break;
    case TypeId::Float:
      os << float_dat_;
      break;
  }
}

}  // namespace cql
