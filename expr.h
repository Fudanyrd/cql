#pragma once

#include <memory>
#include <stdexcept>

#include "tuple.h"
#include "type.h"

namespace cql {

enum ExprType {
  Column,    // column expression.
  Const,     // constant expression
  Unary,     // unary expression of float type
  Binary     // binary expression of float type
};

// abstract arithmetic expression.
class AbstractExpr {
 protected:
  ExprType expr_type_;   // expression type
  TypeId data_type_;     // data type

 public:
  /**
   * @return the data type of the expression.
   */
  virtual auto GetDataType() const -> TypeId { return data_type_; }
  
  /**
   * @return the expression type of expression
   */
  virtual auto GetExprType() const -> ExprType { return expr_type_; }
  
  /**
   * @return the value of a tuple.
   */
  virtual auto Evaluate(const Tuple *tuple) const -> DataBox = 0;

  /**
   * @brief convert a expression to string.
   * This method is generally used for debugging.
   */
  virtual auto toString() const -> std::string = 0;
};
using AbstractExprRef = std::shared_ptr<AbstractExpr>;

class ConstExpr: public AbstractExpr {
 private:
  DataBox data_;
 public:
  ConstExpr() { expr_type_ = ExprType::Const; }

  // in this operator tuple is unused.
  auto Evaluate([[maybe_unused]] const Tuple *tuple) const -> DataBox { return data_; }

  auto toString() const -> std::string {
    switch(data_type_) {
      case INVALID: return "NULL";
      case Char: return data_.getStrValue();
      case Float: return "<float const>";
      default:
        return "<unknown>";
    }
  }
};

// type of unary expression(Only apply to float!)
enum UnaryExprType {
  Minus,               // ~x, get 0 - x.
  Sqrt,                // x^0.5
  Sqr,                 // x^2
  Ln,                  // ln(x)
  Exp,                 // exp(x)
  Sin,                 // sin(x)
  Cos,                 // cos(x)
  Tan,                 // tan(x)
  Asin,                // arcsin(x)
  Acos,                // arccos(x)
  Atan,                // arctan(x)
  invalid
};
// this generally is enough since:
// -x = 0 - x (binary operation)
// pi = acos(-1.0)
// e = exp(1.0)

// unary expression
class UnaryExpr: public AbstractExpr {
 private:
  UnaryExprType optr_type_{invalid};    // operator type.
  AbstractExprRef child_{nullptr};      // has only one child

 public:
  UnaryExpr() { expr_type_ = ExprType::Unary; }
  UnaryExpr(UnaryExprType optr, AbstractExprRef ref): optr_type_(optr), child_(ref) {
    expr_type_ = ExprType::Unary;
  }

  auto Evaluate(const Tuple *tuple) const -> DataBox;
  auto toString() const -> std::string;
};

// type of binary expression(+ applies to string and float; others only apply to float)
enum BinaryExprType {
  add,     // x + y
  sub,     // x - y
  mult,    // x * y
  div,     // x / y
  power,   // x to y
  unknown
};

// binary expression
class BinaryExpr: public AbstractExpr {
 private:
  AbstractExprRef left_child_{nullptr};
  AbstractExprRef right_child_{nullptr};
  BinaryExprType optr_type_{unknown};

 public:
  BinaryExpr() { expr_type_ = ExprType::Binary; }
  BinaryExpr(BinaryExprType tp, AbstractExprRef left, AbstractExprRef right):
    left_child_(left), right_child_(right), optr_type_(tp) {expr_type_ = ExprType::Binary; }

  auto Evaluate(const Tuple *tuple) const -> DataBox;
  auto toString() const -> std::string;
};

// column expression(assosiated with a column with certain kind of tuple)
class ColumnExpr: public AbstractExpr {
 private:
 /** column index of the tuple to extract from.*/
  size_t column_idx_{0U};
 public:
  ColumnExpr() { expr_type_ = ExprType::Column; } 
  ColumnExpr(size_t idx): column_idx_(idx) { expr_type_ = ExprType::Column; }

  auto Evaluate(const Tuple *tuple) const -> DataBox { return tuple->getColumnData(column_idx_); }
  auto toString() const -> std::string;
};

}  // namespace cql;