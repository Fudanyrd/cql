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
    case TypeId::Bool:
      if (dat == "True" || dat == "true") { real_ = true;}
      else { real_ = false; }
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
    case TypeId::Bool:
      if (real_) { os << "True"; }
      else { os << "False"; }
      break;
  }
}

auto DataBox::LessThan(const DataBox &b1, const DataBox &b2) -> DataBox {
  checkType(b1, b2);
  switch(b1.getType()) {
    case TypeId::Char: 
      return {b1.getStrValue() < b2.getStrValue()};
    case TypeId::Float:
      return {b1.getFloatValue() < b2.getFloatValue()};
    case TypeId::Bool:
      return {b1.getBoolValue() < b2.getBoolValue()};
    default:
      return {false};
  }
}
auto DataBox::LessThanOrEqual(const DataBox &b1, const DataBox &b2) -> DataBox {
  checkType(b1, b2);
  switch(b1.getType()) {
    case TypeId::Char: 
      return {b1.getStrValue() <= b2.getStrValue()};
    case TypeId::Float:
      return {b1.getFloatValue() <= b2.getFloatValue()};
    case TypeId::Bool:
      return {b1.getBoolValue() <= b2.getBoolValue()};
    default:
      return {false};
  }
}
auto DataBox::GreaterThan(const DataBox &b1, const DataBox &b2) -> DataBox {
  checkType(b1, b2);
  switch(b1.getType()) {
    case TypeId::Char: 
      return {b1.getStrValue() > b2.getStrValue()};
    case TypeId::Float:
      return {b1.getFloatValue() > b2.getFloatValue()};
    case TypeId::Bool:
      return {b1.getBoolValue() > b2.getBoolValue()};
    default:
      return {false};
  }
}
auto DataBox::GreaterThanOrEqual(const DataBox &b1, const DataBox &b2) -> DataBox {
  checkType(b1, b2);
  switch(b1.getType()) {
    case TypeId::Char: 
      return {b1.getStrValue() >= b2.getStrValue()};
    case TypeId::Float:
      return {b1.getFloatValue() >= b2.getFloatValue()};
    case TypeId::Bool:
      return {b1.getBoolValue() >= b2.getBoolValue()};
    default:
      return {false};
  }
}
auto DataBox::EqualTo(const DataBox &b1, const DataBox &b2) -> DataBox {
  checkType(b1, b2);
  switch(b1.getType()) {
    case TypeId::Char: 
      return {b1.getStrValue() == b2.getStrValue()};
    case TypeId::Float:
      return {b1.getFloatValue() == b2.getFloatValue()};
    case TypeId::Bool:
      return {b1.getBoolValue() == b2.getBoolValue()};
    default:
      return {false};
  }
}
auto DataBox::NotEqualTo(const DataBox &b1, const DataBox &b2) -> DataBox {
  checkType(b1, b2);
  switch(b1.getType()) {
    case TypeId::Char: 
      return {b1.getStrValue() != b2.getStrValue()};
    case TypeId::Float:
      return {b1.getFloatValue() != b2.getFloatValue()};
    case TypeId::Bool:
      return {b1.getBoolValue() != b2.getBoolValue()};
    default:
      return {false};
  }
}

}  // namespace cql
