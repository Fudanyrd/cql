#include <cmath>
#include <sstream>

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

auto DataBox::toStr(const DataBox &b) -> DataBox {
  std::ostringstream oss;  
  oss << b.getFloatValue();
  switch(b.getType()) {
    case TypeId::Char:
      return b;
    case TypeId::Float:
      return DataBox(TypeId::Char, oss.str());
    case TypeId::Bool:
      return DataBox(TypeId::Char, b.getBoolValue() ? "True" : "False");
    case TypeId::INVALID:
      return DataBox(TypeId::Char, "NULL");
  }
}
auto DataBox::toString(const DataBox &b) -> std::string {
  std::ostringstream oss;  
  oss << b.getFloatValue();
  switch(b.getType()) {
    case TypeId::Char:
      return b.getStrValue();
    case TypeId::Float:
      return oss.str();
    case TypeId::Bool:
      return b.getBoolValue() ? "True" : "False";
    case TypeId::INVALID:
      return "NULL";
  }
}
auto DataBox::toFloat(const DataBox &b) -> DataBox {
  switch(b.getType()) {
    case TypeId::Char:
      return DataBox(atof(b.getStrValue().c_str()));
    case TypeId::Float:
      return b;
    case TypeId::Bool:
      return DataBox(b.getBoolValue() ? 1.0 : 0.0);
    case TypeId::INVALID:
      return DataBox(sqrt(-1)); // not a number.
  }
}
auto DataBox::toBool(const DataBox &b) -> DataBox {
  switch(b.getType()) {
    case TypeId::Char:
      return DataBox(!b.getStrValue().empty());
    case TypeId::Float:
      return DataBox(b.getFloatValue() != 0.0);
    case TypeId::Bool:
      return b;
    case TypeId::INVALID:
      return DataBox(false); // not a number.
  }
}
}  // namespace cql
