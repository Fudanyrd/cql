#pragma once
#include <cmath>
#include <ios>
#include <iostream>
#include <stdexcept>

namespace cql {

// types cql language currently support.
enum TypeId {
  Float,     // double in cpp.
  Char,      // varchar in sql.
  Bool,      // true or false.
  INVALID    // invalid type.
};

// databox class
class DataBox {
 private:
  std::string str_dat_;          // if it has char type...
  double float_dat_{0.0};        // if it has type float...
  bool real_{false};             // if it has type bool...
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
   * @brief create a box with type bool.
   */
  DataBox(bool real) {
    real_ = real;
    type_ = TypeId::Bool;
  }

  /**
   * @param tp: type of the data.
   * @param dat: data used to initialize the databox.
   */
  DataBox(TypeId tp, const std::string &dat);

  auto getType() const -> TypeId { return type_; }
  auto getFloatValue() const -> double { return float_dat_; }
  auto getStrValue() const -> std::string { return str_dat_; }
  auto getBoolValue() const -> bool { return real_; }

  /**
   * @throws std::domain_error if two boxes hold different types.
   */
  static void checkType(const DataBox &b1, const DataBox &b2) {
    if (b1.getType() != b2.getType()) {
      throw std::domain_error("comparison between different types?? Impossible!");
    }
  }
  
  ///////////////////////////
  // DataBox Comparison
  ///////////////////////////
  static auto LessThan(const DataBox &b1, const DataBox &b2) -> DataBox;
  static auto LessThanOrEqual(const DataBox &b1, const DataBox &b2) -> DataBox;
  static auto GreaterThan(const DataBox &b1, const DataBox &b2) -> DataBox;
  static auto GreaterThanOrEqual(const DataBox &b1, const DataBox &b2) -> DataBox;
  static auto EqualTo(const DataBox &b1, const DataBox &b2) -> DataBox;
  static auto NotEqualTo(const DataBox &b1, const DataBox &b2) -> DataBox;

  void printTo(std::ostream &os) const;
};

}  // namespace cql