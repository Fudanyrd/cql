#pragma once
#include <cmath>
#include <ios>
#include <iostream>

namespace cql {

// types cql language currently support.
enum TypeId {
  Float,     // double in cpp.
  Char,      // varchar in sql.
  INVALID    // invalid type.
};

// databox class
class DataBox {
 private:
  std::string str_dat_;          // if it has char type...
  double float_dat_{0.0};        // if it has type float...
  TypeId type_{INVALID};

 public:
  DataBox() = default;
  /**
   * @brief create databox using float type
   */
  DataBox(double d) {
    float_dat_ = d;
    type_ = TypeId::Float;
  }
  /**
   * @param tp: type of the data.
   * @param dat: data used to initialize the databox.
   */
  DataBox(TypeId tp, const std::string &dat);

  auto getType() const -> TypeId { return type_; }
  auto getFloatValue() const -> double { return float_dat_; }
  auto getStrValue() const -> std::string { return str_dat_; }

  void printTo(std::ostream &os) const;
};

}  // namespace cql