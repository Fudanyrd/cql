#include <cmath>
#include <stack>
#include <stdexcept>

#include "expr.h"

namespace cql {

/**********************************************************
 *                     UnaryExpr
 **********************************************************/

auto UnaryExpr::Evaluate(const Tuple *tuple, VariableManager *var_mgn, size_t idx) const -> DataBox {
  if (!static_cast<bool>(child_)) {
    throw std::domain_error("child is null?? impossible!");
  }

  DataBox child_val = child_->Evaluate(tuple, var_mgn, idx);
  if (child_val.getType() == TypeId::INVALID) {
    // it means that one variable index is out of bounds...
    return DataBox(TypeId::INVALID, "");
  }
  const double chd = child_val.getFloatValue();
  if (child_val.getType() == TypeId::Char) {
    throw std::domain_error("calling unary operator on string?? impossible!");
  }
  if (child_val.getType() == TypeId::Bool) {
    if (optr_type_ == UnaryExprType::Not) {
      return {!child_val.getBoolValue()};
    }
    throw std::domain_error("unary operator on bool other than not?? Impossible!");
  }
  double res = 0.0;
  switch(optr_type_) {
    case UnaryExprType::Minus:
      res = -chd;
      break;
    case UnaryExprType::Sgn:
      res = chd == 0.0 ? 0.0 : (chd > 0.0 ? 1.0 : -1.0);
      break;
    case UnaryExprType::Abs:
      res = fabs(chd);
      break;
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
    case UnaryExprType::Not:
      throw std::domain_error("not operator on a float?? Impossible!");
      break;
    default:
      throw std::domain_error("invalid unary operator type!");
  }

  return DataBox(res);
}

auto UnaryExpr::toString() const -> std::string {
  std::string res;
  switch(optr_type_) {
    case UnaryExprType::Minus:
      res += "~(";
      break;
    case UnaryExprType::Sgn:
      res += "sgn(";
      break;
    case UnaryExprType::Abs:
      res += "abs(";
      break;
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
    case UnaryExprType::Not:
      res += "not(";
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
auto BinaryExpr::Evaluate(const Tuple *tuple, VariableManager *var_mgn, size_t idx) const -> DataBox {
  if (!(static_cast<bool>(left_child_) && static_cast<bool>(right_child_))) {
    throw std::domain_error("left or right child is null");
  }

  auto left_box = left_child_->Evaluate(tuple, var_mgn, idx);
  auto right_box = right_child_->Evaluate(tuple, var_mgn, idx);
  if (left_box.getType() == TypeId::INVALID || right_box.getType() == TypeId::INVALID) {
    return DataBox(TypeId::INVALID, "");
  }

  switch(optr_type_) {
    case BinaryExprType::LessThan:
      return DataBox::LessThan(left_box, right_box);
    case BinaryExprType::LessThanOrEqual:
      return DataBox::LessThanOrEqual(left_box, right_box);
    case BinaryExprType::GreaterThan:
      return DataBox::GreaterThan(left_box, right_box);
    case BinaryExprType::GreaterThanOrEqual:
      return DataBox::GreaterThanOrEqual(left_box, right_box);
    case BinaryExprType::EqualTo:
      return DataBox::EqualTo(left_box, right_box);
    case BinaryExprType::NotEqualTo:
      return DataBox::NotEqualTo(left_box, right_box);
    // others will be handled later.
  }

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
  if (left_box.getType() == TypeId::Bool) {
    bool left_r = left_box.getBoolValue();
    bool right_r = right_box.getBoolValue();
    bool res;
    switch(optr_type_) {
      case BinaryExprType::And: 
        res = left_r & right_r;
        break;
      case BinaryExprType::Or: 
        res = left_r | right_r;
        break;
      case BinaryExprType::Xor:
        res = left_r ^ right_r;
        break;
      default:
        throw std::domain_error("wanna operation on bool other than ^&|? Impossible!");
    }
    return {res};
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
    case BinaryExprType::mod:  // little significant bits are discarded!
      res = static_cast<double>(static_cast<long>(left_val) % static_cast<long>(right_val));
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
    case BinaryExprType::And: case BinaryExprType::Or: case BinaryExprType::Xor:
      throw std::domain_error("doing logic operation(&|^) on two float?? Impossible!");
      break;
    default:
      throw std::domain_error("unrecognizable binary operation on float?? Impossible!");
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
    case BinaryExprType::mod:
      res += ")%(";
      break;
    case BinaryExprType::div:
      res += ")/(";
      break;
    case BinaryExprType::power:
      res += ")^(";
      break;
    case BinaryExprType::And:
      res += ") and (";
      break;
    case BinaryExprType::Or:
      res += ") or (";
      break;
    case BinaryExprType::Xor:
      res += ") xor (";
      break;
    case BinaryExprType::LessThan:
      res += ") < (";
      break;
    case BinaryExprType::LessThanOrEqual:
      res += ") <= (";
      break;
    case BinaryExprType::GreaterThan:
      res += ") > (";
      break;
    case BinaryExprType::GreaterThanOrEqual:
      res += ") >= (";
      break;
    case BinaryExprType::EqualTo:
      res += ") == (";
      break;
    case BinaryExprType::NotEqualTo:
      res += ") != (";
      break;

    default:
      // throw std::domain_error("unrecognizable binary operation on float");
      res += ")<unknown operator>(";
  }

  res += right_child_->toString();
  res += ")";
  return res;
}

}  // namespace cql
