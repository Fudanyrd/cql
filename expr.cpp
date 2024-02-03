#include <cmath>
#include <stack>
#include <stdexcept>

#include "expr.h"

namespace cql {

/**********************************************************
 *                     UnaryExpr
 **********************************************************/

auto UnaryExpr::Evaluate(const Tuple *tuple) const -> DataBox {
  if (!static_cast<bool>(child_)) {
    throw std::domain_error("child is null?? impossible!");
  }

  DataBox child_val = child_->Evaluate(tuple);
  const double chd = child_val.getFloatValue();
  if (!child_val.getType() == TypeId::Float) {
    throw std::domain_error("calling unary operator on string?? impossible!");
  }
  double res = 0.0;
  switch(optr_type_) {
    case UnaryExprType::Sqrt: 
      res = pow(chd, 0.5);
      break;
    case UnaryExprType::Sqr:
      res = chd * chd;
      break;
    case UnaryExprType::Ln:
      res = log(chd);
      break;
    case UnaryExprType::Exp:
      res = exp(chd);
      break;
    case UnaryExprType::Sin:
      res = sin(chd);
      break;
    case UnaryExprType::Cos:
      res = cos(chd);
      break;
    case UnaryExprType::Tan:
      res = tan(chd);
      break;
    case UnaryExprType::Asin:
      res = asin(chd);
      break;
    case UnaryExprType::Acos:
      res = acos(chd);
      break;
    case UnaryExprType::Atan:
      res = atan(chd);
      break;
    default:
      throw std::domain_error("invalid unary operator type!");
  }

  return DataBox(res);
}

auto UnaryExpr::toString() const -> std::string {
  std::string res;
  switch(optr_type_) {
    case UnaryExprType::Sqrt: 
      res += "sqrt(";
      break;
    case UnaryExprType::Sqr:
      res += "sqr(";
      break;
    case UnaryExprType::Ln:
      res += "ln(";
      break;
    case UnaryExprType::Exp:
      res += "exp(";
      break;
    case UnaryExprType::Sin:
      res += "sin(";
      break;
    case UnaryExprType::Cos:
      res += "cos(";
      break;
    case UnaryExprType::Tan:
      res += "tan(";
      break;
    case UnaryExprType::Asin:
      res += "arcsin(";
      break;
    case UnaryExprType::Acos:
      res += "arccos(";
      break;
    case UnaryExprType::Atan:
      res += "arctan(";
      break;
    default:
      throw std::domain_error("invalid unary operator type!");
  }
  res += child_->toString();
  return res + ")";
}

/**********************************************************
 *                     BinaryExpr
 **********************************************************/
auto BinaryExpr::Evaluate(const Tuple *tuple) const -> DataBox {
  if (!(static_cast<bool>(left_child_) && static_cast<bool>(right_child_))) {
    throw std::domain_error("left or right child is null");
  }

  auto left_box = left_child_->Evaluate(tuple);
  auto right_box = right_child_->Evaluate(tuple);
  if (left_box.getType() != right_box.getType()) {
    throw std::domain_error("left and right return type is different?? Impossible!");
  } 

  if (left_box.getType() == TypeId::Char) {
    // OK, string type.
    if (optr_type_ != BinaryExprType::add) {
      throw std::domain_error("wanna operation on string other than sum? Impossible!");
    }
    return DataBox(TypeId::Char, left_box.getStrValue() + right_box.getStrValue());
  }
  
  // OK, float type.
  double left_val = left_box.getFloatValue();
  double right_val = right_box.getFloatValue();
  double res = 0.0;
  switch(optr_type_) {
    case BinaryExprType::add:
      res = left_val + right_val;
      break;
    case BinaryExprType::sub:
      res = left_val - right_val;
      break;
    case BinaryExprType::mult:
      res = left_val * right_val;
      break;
    case BinaryExprType::div:
      res = left_val / right_val;
      break;
    case BinaryExprType::power:
      res = pow(left_val, right_val);
      break;
    default:
      throw std::domain_error("unrecognizable binary operation on float");
  }

  return DataBox(res);
}

auto BinaryExpr::toString() const -> std::string {
  std::string res = "(" + left_child_->toString();

  switch(optr_type_) {
    case BinaryExprType::add:
      res += ")+(";
      break;
    case BinaryExprType::sub:
      res += ")-(";
      break;
    case BinaryExprType::mult:
      res += ")*(";
      break;
    case BinaryExprType::div:
      res += ")/(";
      break;
    case BinaryExprType::power:
      res += ")^(";
      break;
    default:
      // throw std::domain_error("unrecognizable binary operation on float");
      res += ")<unknown operator>(";
  }

  res += right_child_->toString();
  res += ")";
  return res;
}

/**********************************************************
 *                     ColumnExpr
 **********************************************************/
auto ColumnExpr::toString() const -> std::string {
  std::string res = "#";
  size_t col = column_idx_;
  if (col == 0) {
    res.push_back('0');
    return res;
  }

  std::stack<char> temp;
  while (col > 0) {
    temp.push(static_cast<int>(col % 10U) + '0');
    col = col / 10U;
  }

  while (!temp.empty()) {
    res.push_back(temp.top());
    temp.pop();
  }
  return res;
}
}  // namespace cql