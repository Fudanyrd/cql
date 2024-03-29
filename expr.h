#pragma once

#include <memory>
#include <stdexcept>

#include "tuple.h"
#include "type.h"
#include "variable_manager.h"

namespace cql {

/**
 * NOTE: all variables in cql language begins with '@'.
 * so as to simplify works done by compiler.
 */
enum ExprType {
  Aggregate,
  Column,    // column expression.
  Const,     // constant expression
  Unary,     // unary expression of float type
  Binary,    // binary expression of float type
  Variable   // TODO(Fudanyrd): add variable type.
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
  virtual ~AbstractExpr() = default;
  
  /**
   * @return the expression type of expression
   */
  virtual auto GetExprType() const -> ExprType { return expr_type_; }

  /**
   * @return the cloned node ref without cloning children.
   */
  virtual auto Clone() const -> std::shared_ptr<AbstractExpr> = 0;
  
  /**
   * @return the value of a tuple.
   * If it is a variable, return the idx(th) element of it.
   * If out of bounds, return a data box with invalid type.
   */
  virtual auto Evaluate(const Tuple *tuple, VariableManager *var_mgn, size_t idx) 
    const -> DataBox = 0;

  /**
   * @brief convert a expression to string.
   * This method is generally used for debugging.
   */
  virtual auto toString() const -> std::string = 0;
};
using AbstractExprRef = std::shared_ptr<AbstractExpr>;

class ConstExpr: public AbstractExpr {
 public:
  DataBox data_;

  ConstExpr() { expr_type_ = ExprType::Const; }
  ConstExpr(const DataBox &box) {
    expr_type_ = ExprType::Const;
    data_ = box;
  }
  ConstExpr(TypeId tp, const std::string &dat): data_(tp, dat) {
    expr_type_ = ExprType::Const;
  }

  auto Clone() const -> AbstractExprRef override {
    return std::make_shared<ConstExpr>(ConstExpr(data_));
  }

  // in this operator tuple is unused.
  auto Evaluate(const Tuple *tuple, VariableManager *var_mgn, size_t idx) const -> DataBox { return data_; }

  auto toString() const -> std::string { return DataBox::toString(data_); }
};

// type of unary expression(Only apply to float!)
enum UnaryExprType {
  Minus,               // ~x, get 0 - x.
  Sgn,                 // sgn(x).
  Abs,                 // abs(x)
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
  Not,                 // logic not
  ToStr,               // casting to string
  ToFloat,             // casting to float
  ToBool,              // casting to boolean
  invalid
};
// this generally is enough since:
// -x = 0 - x (binary operation)
// pi = acos(-1.0)
// e = exp(1.0)

// unary expression
class UnaryExpr: public AbstractExpr {
 public:
  UnaryExprType optr_type_{invalid};    // operator type.
  AbstractExprRef child_{nullptr};      // has only one child

  UnaryExpr() { expr_type_ = ExprType::Unary; }
  UnaryExpr(UnaryExprType optr, AbstractExprRef ref = nullptr): optr_type_(optr), child_(ref) {
    expr_type_ = ExprType::Unary;
  }

  auto Clone() const -> AbstractExprRef override {
    return std::make_shared<UnaryExpr>(UnaryExpr(optr_type_, nullptr));
  }

  auto Evaluate(const Tuple *tuple, VariableManager *var_mgn, size_t idx) const -> DataBox;
  auto toString() const -> std::string;
};

// important note on 'in' operator:
// "not false in @var"
//   is equivalent to:
// "not (false in @var)"

// type of binary expression(+ applies to string and float; others only apply to float)
enum BinaryExprType {
  add,     // x + y
  sub,     // x - y
  mod,     // (int)x % (int)y
  mult,    // x * y
  div,     // x / y
  power,   // x to y
  And,     // x and y
  Or,      // x or y
  Xor,     // x xor y
  LessThan,             // <
  LessThanOrEqual,      // <=
  GreaterThan,          // >
  GreaterThanOrEqual,   // >=
  EqualTo,              // =
  NotEqualTo,           // !=
  in,                   // a in @var
  unknown
};

// binary expression
class BinaryExpr: public AbstractExpr {
 public:
  AbstractExprRef left_child_{nullptr};
  AbstractExprRef right_child_{nullptr};
  BinaryExprType optr_type_{unknown};

  BinaryExpr() { expr_type_ = ExprType::Binary; }
  BinaryExpr(BinaryExprType tp, AbstractExprRef left, AbstractExprRef right):
    left_child_(left), right_child_(right), optr_type_(tp) { expr_type_ = ExprType::Binary; }

  auto Clone() const -> AbstractExprRef override {
    return std::make_shared<BinaryExpr>(BinaryExpr(optr_type_, nullptr, nullptr));
  }

  auto Evaluate(const Tuple *tuple, VariableManager *var_mgn, size_t idx) const -> DataBox;
  auto toString() const -> std::string;
};

// aggregation types
enum AggregateType {
  Agg,
  Count,
  Max,
  Min,
  Sum
};

// aggregation expression
class AggregateExpr: public AbstractExpr {
 public:
  AggregateType agg_type_;
  AbstractExprRef child_;
  AggregateExpr(AggregateType agg_tp, AbstractExprRef child) {
    this->expr_type_ = ExprType::Aggregate;
    this->agg_type_ = agg_tp;
    this->child_ = child;
  }

  auto Clone() const -> AbstractExprRef override {
    return std::make_shared<AggregateExpr>(AggregateExpr(agg_type_, nullptr));
  }

  auto Evaluate(const Tuple *tuple, VariableManager *var_mgn, size_t idx) const -> DataBox override {
    auto schema_ptr = tuple->getSchema();
    std::string column_name_ = this->toString();
    for (size_t i = 0; i < schema_ptr->getNumCols(); ++i) {
      // std::cout << schema_ptr->getNumCols();
      if (column_name_ == schema_ptr->getColumn(i).second) {
        return tuple->getColumnData(i); 
      }
    }
    throw std::domain_error(("unable to recognize column name " + column_name_ + "?? Impossible!").c_str());
  }

  auto toString() const -> std::string override;
};

// column expression(assosiated with a column with certain kind of tuple)
// all column name has prefix '#'.
class ColumnExpr: public AbstractExpr {
 public:
 /** column index of the tuple to extract from.*/
  size_t column_idx_{0U};
  /** Column name of the column('#' included). */
  std::string column_name_;
 public:
  ColumnExpr() { expr_type_ = ExprType::Column; } 
  ColumnExpr(size_t idx, const std::string &col_nm): column_idx_(idx), column_name_(col_nm) { 
    expr_type_ = ExprType::Column; 
  }

  auto Clone() const -> AbstractExprRef override {
    return std::make_shared<ColumnExpr>(ColumnExpr(column_idx_, column_name_));
  }

  auto Evaluate(const Tuple *tuple, [[maybe_unused]] VariableManager *var_mgn, size_t idx) const -> DataBox { 
    auto schema_ptr = tuple->getSchema();
    for (size_t i = 0; i < schema_ptr->getNumCols(); ++i) {
      if (column_name_ == schema_ptr->getColumn(i).second) {
        return tuple->getColumnData(i); 
      }
    }
    throw std::domain_error(("unable to recognize column name " + column_name_ + "?? Impossible!").c_str());
  }
  auto toString() const -> std::string { return column_name_; }
};

// variable expression
class VariableExpr: public AbstractExpr {
 public:
  std::string var_name_{"@"};    // variable name(including prefix '@')

  VariableExpr() { expr_type_ = ExprType::Variable; }
  VariableExpr(const std::string &name): var_name_(name) {
    expr_type_ = ExprType::Variable;
  }

  auto Clone() const -> AbstractExprRef override {
    return std::make_shared<VariableExpr>(VariableExpr(var_name_));
  }

  // currently not able to do this...
  auto Evaluate(const Tuple *tuple, VariableManager *var_mgn, size_t idx) const -> DataBox {
    cqlAssert(static_cast<bool>(var_mgn), "variable manager is nullptr");
    const std::vector<DataBox> &data = var_mgn->Retrive(var_name_);
    return idx >= data.size() ? DataBox(TypeId::INVALID, "") : data[idx];
  }
  auto toString() const -> std::string { return var_name_; }
};

}  // namespace cql;