#include <stack>
#include <unordered_map>

#include "expr_util.h"

namespace cql {

/************************************************
 * Backgroud knowledge
 * 
 * (1) in-stack priority(isp) of operators:
 * NOTE: function calls usually have very high 
 * priorities.
 * Defined BELOW
 ************************************************/
const std::unordered_map<std::string, int> isp_table = {
  {"(", 0},
  // mult, div, power
  {"*", 12},
  {"^", 12},
  {"/", 12},
  {"%", 12},
  // add, sub
  {"+", 10},
  {"-", 10},
  // comparison
  {"<", 8},
  {">", 8},
  {"<=", 8},
  {">=", 8},
  {"=", 8},
  // in
  {"in", 4},
  // not
  {"not", 3},
  // and, or, xor
  {"and", 2},
  {"or", 2},
  {"xor", 2},
  // right parentese
  {")", 15}
};
/**
 * @return the in-stack priority of a operator.
 */
auto isp(const std::string &optr) -> int {
  auto iter = isp_table.find(optr);
  // if not match any, probably a unary operator like:
  // sin, cos, tan, ...
  return iter == isp_table.end() ? 14 : iter->second;
}

/************************************************
 * Backgroud knowledge
 * 
 * (2) in-coming priority(icp) of operators:
 * Defined BELOW
 ************************************************/
const std::unordered_map<std::string, int> icp_table = {
  {"(", 15},
  // mult, div, power
  {"*", 11},
  {"^", 11},
  {"/", 11},
  {"%", 11},
  // add, sub
  {"+", 9},
  {"-", 9},
  // comparison
  {"<", 7},
  {">", 7},
  {"<=", 7},
  {">=", 7},
  {"=", 7},
  // not
  {"not", 6},
  // in
  {"in", 5},
  // and, or, xor
  {"and", 1},
  {"or",  1},
  {"xor", 1},
  // right parentese
  {")", 0}
};
/**
 * @return the in-coming priority of a operator.
 */
auto icp(const std::string &optr) -> int {
  auto iter = icp_table.find(optr);
  // if not match any, probably a unary operator like:
  // sin, cos, tan, ...
  return iter == isp_table.end() ? 13 : iter->second;
}

/**
 * @return not nullptr if the word is a constant expression.
 */
auto getConstExpr(const std::string &word) -> AbstractExprRef {
  if (word.empty()) {
    return nullptr;
  }
  if (word == "True" || word == "true") {
    // true
    return std::make_shared<ConstExpr>(ConstExpr(TypeId::Bool, word));
  }
  if (word == "False" || word == "false") {
    // false
    return std::make_shared<ConstExpr>(ConstExpr(TypeId::Bool, word));
  }
  if (word[0] == '\'' && word[word.size() - 1] == '\'') {
    // string literal
    return std::make_shared<ConstExpr>(ConstExpr(TypeId::Char, word.substr(1, word.size() - 2)));
  }
  // check if it is float...
  for (auto ch : word) {
    if (ch == '.' || (ch >= '0' && ch <= '9')) {
      continue;
    }
    return nullptr;
  }
  // OK, has float type.
  return std::make_shared<ConstExpr>(ConstExpr(TypeId::Float, word));
}

/**
 * @return the variable expression node.
 * nullptr if not an variable type.
 */
auto getVariableExpr(const std::string &word) -> AbstractExprRef {
  if (word.empty() || word[0] != '@') {
    // not a variable word!
    return nullptr;
  }
  return std::make_shared<VariableExpr>(VariableExpr(word));
}
/**
 * @return the column expression node.
 * nullptr if not an column type.
 */
auto getColumnExpr(const std::string &word) -> AbstractExprRef {
  if (word.empty() || word[0] != '#') {
    // not a variable word!
    return nullptr;
  }
  std::string actual;
  for (size_t i = 1; i < word.size(); ++i) { actual.push_back(word[i]); }
  return std::make_shared<ColumnExpr>(ColumnExpr(0U, actual));
}

auto MinusOperator() -> AbstractExprRef  {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Minus, nullptr));
}
auto EqualToOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::EqualTo, nullptr, nullptr));
}
auto LessThanOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::LessThan, nullptr, nullptr));
}
auto GreaterThanOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::GreaterThan, nullptr, nullptr));
}
auto AddOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::add, nullptr, nullptr));
}
auto SubOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::sub, nullptr, nullptr));
}
auto MultOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::mult, nullptr, nullptr));
}
auto ModOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::mod, nullptr, nullptr));
}
auto DivOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::div, nullptr, nullptr));
}
auto PowerOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::power, nullptr, nullptr));
}
auto LessThanOrEqualOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::LessThanOrEqual, nullptr, nullptr));
}
auto GreaterThanOrEqualOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::GreaterThanOrEqual, nullptr, nullptr));
}
auto AndOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::And, nullptr, nullptr));
}
auto OrOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::Or, nullptr, nullptr));
}
auto XorOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::Xor, nullptr, nullptr));
}
auto InOperator() -> AbstractExprRef {
  return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::in, nullptr, nullptr));
}
auto ToBoolOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::ToBool, nullptr));
}
auto ToFloatOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::ToFloat, nullptr));
}
auto ToStrOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::ToStr, nullptr));
}
auto NotOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Not, nullptr));
}
auto ExpOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Exp));
}
auto LnOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Ln));
}
auto SinOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Sin));
}
auto CosOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Cos));
}
auto TanOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Tan));
}
auto SgnOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Sgn));
}
auto AbsOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Abs));
}
auto AsinOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Asin));
}
auto AcosOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Acos));
}
auto AtanOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Atan));
}
auto SqrtOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Sqrt));
}
auto SqrOperator() -> AbstractExprRef {
  return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Sqr));
}
auto AggOperator() -> AbstractExprRef {
  return std::make_shared<AggregateExpr>(AggregateExpr(AggregateType::Agg, nullptr));
}
auto CountOperator() -> AbstractExprRef {
  return std::make_shared<AggregateExpr>(AggregateExpr(AggregateType::Count, nullptr));
}
auto MaxOperator() -> AbstractExprRef {
  return std::make_shared<AggregateExpr>(AggregateExpr(AggregateType::Max, nullptr));
}
auto MinOperator() -> AbstractExprRef {
  return std::make_shared<AggregateExpr>(AggregateExpr(AggregateType::Min, nullptr));
}
auto SumOperator() -> AbstractExprRef {
  return std::make_shared<AggregateExpr>(AggregateExpr(AggregateType::Sum, nullptr));
}
 
typedef AbstractExprRef (*ExprFunc)();  // expr functions
std::unordered_map<std::string, ExprFunc> operator_factory = {
  {"~", MinusOperator},
  {"=", EqualToOperator},
  {"<", LessThanOperator},
  {">", GreaterThanOperator},
  {"+", AddOperator},
  {"-", SubOperator},
  {"*", MultOperator},
  {"%", ModOperator},
  {"/", DivOperator},
  {"^", PowerOperator},
  {"<=", LessThanOrEqualOperator},
  {">=", GreaterThanOrEqualOperator},
  {"and", AndOperator},
  {"or", OrOperator},
  {"xor", XorOperator},
  {"in", InOperator},
  {"tobool", ToBoolOperator},
  {"tofloat", ToFloatOperator},
  {"tostr", ToStrOperator},
  {"not", NotOperator},
  {"exp", ExpOperator},
  {"ln", LnOperator},
  {"sin", SinOperator},
  {"cos", CosOperator},
  {"tan", TanOperator},
  {"sgn", SgnOperator},
  {"abs", AbsOperator},
  {"asin", AsinOperator},
  {"acos", AcosOperator},
  {"atan", AtanOperator},
  {"sqrt", SqrtOperator},
  {"sqr", SqrOperator},
  {"agg", AggOperator},
  {"count", CountOperator},
  {"max", MaxOperator},
  {"min", MinOperator},
  {"sum", SumOperator}
};

/**
 * @return the operator expression of a word.
 * nullptr if failed to match.
 */
auto getOperator(const std::string &word) -> AbstractExprRef {
  if (word.empty()) { return nullptr; }
  // simplify the work...
  if (word[0] == '#') { return nullptr; }

  auto iter = operator_factory.find(word);
  if (iter != operator_factory.end()) {
    ExprFunc func = iter->second;
    return func();
  }

  // matching failed.
  return nullptr;
}

auto toPostOrder(const std::vector<std::string> &words) -> std::vector<AbstractExprRef> {
  // psuedo code of how to achieve this:
  // if word is const expr; then output.
  // else: # op is the top of stack
  //    if icp(word) > isp(op); then push word in stack.
  //    if icp(word) < isp(op); then pop and output.
  //    else pop and don't output.
  std::stack<std::string> operators;       // operator stack.
  std::vector<AbstractExprRef> expr_refs;  // store the result.
  for (size_t i = 0; i < words.size(); ++i) {
    const std::string &word = words[i];
    auto current = getConstExpr(word);
    if (static_cast<bool>(current)) {
      // OK, is constant expression.
      expr_refs.push_back(current);
      // ++i;
      continue;
    }

    current = getVariableExpr(word);
    if (static_cast<bool>(current)) {
      // OK, is a variable.
      expr_refs.push_back(current);
      // ++i;
      continue;
    }

    current = getColumnExpr(word);
    if (static_cast<bool>(current)) {
      // OK, is column from a table...
      expr_refs.push_back(current);
      // ++i;
      continue;
    }

    current = getOperator(word);
    if (static_cast<bool>(current) || word == "(" || word == ")") {
      // OK, is a variable.
      if (!operators.empty()) {
        int in_come = icp(word);              // icp(word) in psuedo code
        int in_stack = isp(operators.top());  // isp(op) in psuedo code
        if (in_come > in_stack) {
          operators.push(word);
          // ++i;
        } else {
          if (in_come < in_stack) {
            if (!(operators.top() == "(" || operators.top() == ")")) {
              expr_refs.push_back(getOperator(operators.top()));
            }
            operators.pop();
            // operators.push(word);
            --i;  // --i; ++i; <=> nop
          } else {
            // to this point, only one circumstance is possible:
            // operators.top() == "(" and word = ")".
            operators.pop();
            // ++i;
          }
        }

      } else {
        operators.push(word);
        // ++i;
      }
    } else {
      std::string error_msg = "ERROR: unable to recognize word " + word;
      throw std::domain_error(error_msg.c_str());
    }

  }

  while (!operators.empty()) {
    if (operators.top() != "(" && operators.top() != ")") {
      expr_refs.push_back(getOperator(operators.top()));
    }
    operators.pop();
  }
  return expr_refs;
}

auto toPostOrder(const std::vector<std::string> &words, size_t begin, size_t end) 
  -> std::vector<AbstractExprRef> {
  // psuedo code of how to achieve this:
  // if word is const expr; then output.
  // else: # op is the top of stack
  //    if icp(word) > isp(op); then push word in stack.
  //    if icp(word) < isp(op); then pop and output.
  //    else pop and don't output.
  std::stack<std::string> operators;       // operator stack.
  std::vector<AbstractExprRef> expr_refs;  // store the result.
  // for (size_t i = 0; i < words.size(); ++i) 
  for (size_t i = begin; i < end; ++i) {
    const std::string &word = words[i];
    auto current = getConstExpr(word);
    if (static_cast<bool>(current)) {
      // OK, is constant expression.
      expr_refs.push_back(current);
      // ++i;
      continue;
    }

    current = getVariableExpr(word);
    if (static_cast<bool>(current)) {
      // OK, is a variable.
      expr_refs.push_back(current);
      // ++i;
      continue;
    }

    current = getColumnExpr(word);
    if (static_cast<bool>(current)) {
      // OK, is column from a table...
      expr_refs.push_back(current);
      // ++i;
      continue;
    }

    current = getOperator(word);
    if (static_cast<bool>(current) || word == "(" || word == ")") {
      // OK, is a variable.
      if (!operators.empty()) {
        int in_come = icp(word);              // icp(word) in psuedo code
        int in_stack = isp(operators.top());  // isp(op) in psuedo code
        if (in_come > in_stack) {
          operators.push(word);
          // ++i;
        } else {
          if (in_come < in_stack) {
            if (!(operators.top() == "(" || operators.top() == ")")) {
              expr_refs.push_back(getOperator(operators.top()));
            }
            operators.pop();
            // operators.push(word);
            --i;  // --i; ++i; <=> nop
          } else {
            // to this point, only one circumstance is possible:
            // operators.top() == "(" and word = ")".
            operators.pop();
            // ++i;
          }
        }

      } else {
        operators.push(word);
        // ++i;
      }
    } else {
      std::string error_msg = "ERROR: unable to recognize word " + word;
      throw std::domain_error(error_msg.c_str());
    }

  }

  while (!operators.empty()) {
    if (operators.top() != "(" && operators.top() != ")") {
      expr_refs.push_back(getOperator(operators.top()));
    }
    operators.pop();
  }
  return expr_refs;
}

auto toExprRef(const std::vector<std::string> &words) -> AbstractExprRef {
  std::vector<AbstractExprRef> refs = toPostOrder(words);
  std::stack<AbstractExprRef> operands;
  for (const auto &ref : refs) {
    switch(ref->GetExprType()) {
      case ExprType::Unary: {
        if (operands.empty()) {
          throw std::domain_error("unary expr missing operand?? Impossible!");
        }
        auto unary_ptr = dynamic_cast<UnaryExpr *>(ref.get());
        unary_ptr->child_ = operands.top();
        operands.pop();
        operands.push(std::make_shared<UnaryExpr>(UnaryExpr(unary_ptr->optr_type_, unary_ptr->child_)));
        break;
      }
      case ExprType::Aggregate: {
        if (operands.empty()) {
          throw std::domain_error("aggregate expr missing operand?? Impossible!");
        }
        auto unary_ptr = dynamic_cast<AggregateExpr *>(ref.get());
        unary_ptr->child_ = operands.top();
        operands.pop();
        operands.push(std::make_shared<AggregateExpr>(AggregateExpr(unary_ptr->agg_type_, unary_ptr->child_)));
        break;
      }

      case ExprType::Binary: {
        if (operands.size() < 2) {
          throw std::domain_error("binary expr missing one or both operand?? Impossible!");
        }
        auto binary_ptr = dynamic_cast<BinaryExpr *>(ref.get());
        binary_ptr->right_child_ = operands.top();
        operands.pop();
        binary_ptr->left_child_ = operands.top();
        operands.pop();
        operands.push(std::make_shared<BinaryExpr>(BinaryExpr(binary_ptr->optr_type_, 
                                                    binary_ptr->left_child_, binary_ptr->right_child_)));
        break;
      }
      // maybe constants, variables, columns.
      default:
        operands.push(ref);
    }
  }

  return operands.top();
}

auto toExprRef(const std::vector<std::string> &words, size_t begin, size_t end) -> AbstractExprRef {
  std::vector<AbstractExprRef> refs = toPostOrder(words, begin, end);
  std::stack<AbstractExprRef> operands;
  for (const auto &ref : refs) {
    switch(ref->GetExprType()) {
      case ExprType::Unary: {
        if (operands.empty()) {
          throw std::domain_error("unary expr missing operand?? Impossible!");
        }
        auto unary_ptr = dynamic_cast<UnaryExpr *>(ref.get());
        unary_ptr->child_ = operands.top();
        operands.pop();
        operands.push(std::make_shared<UnaryExpr>(UnaryExpr(unary_ptr->optr_type_, unary_ptr->child_)));
        break;
      }
      case ExprType::Aggregate: {
        if (operands.empty()) {
          throw std::domain_error("aggregate expr missing operand?? Impossible!");
        }
        auto unary_ptr = dynamic_cast<AggregateExpr *>(ref.get());
        unary_ptr->child_ = operands.top();
        operands.pop();
        operands.push(std::make_shared<AggregateExpr>(AggregateExpr(unary_ptr->agg_type_, unary_ptr->child_)));
        break;
      }

      case ExprType::Binary: {
        if (operands.size() < 2) {
          throw std::domain_error("binary expr missing one or both operand?? Impossible!");
        }
        auto binary_ptr = dynamic_cast<BinaryExpr *>(ref.get());
        binary_ptr->right_child_ = operands.top();
        operands.pop();
        binary_ptr->left_child_ = operands.top();
        operands.pop();
        operands.push(std::make_shared<BinaryExpr>(BinaryExpr(binary_ptr->optr_type_, 
                                                    binary_ptr->left_child_, binary_ptr->right_child_)));
        break;
      }
      // maybe constants, variables, columns.
      default:
        operands.push(ref);
    }
  }

  return operands.top();
}

auto isConstExpr(const AbstractExprRef &root) -> bool {
  if (!static_cast<bool>(root)) {
    throw std::domain_error("trying to tell if a null expr tree is const?? Impossible!");
  }
  const UnaryExpr *unary_ptr;
  const BinaryExpr *bin_ptr;
  const AggregateExpr *agg_ptr;
  switch(root->GetExprType()) {
    case ExprType::Column: case ExprType::Variable:
      return false;
    case ExprType::Const:
      return true;
    case ExprType::Unary:
      unary_ptr = dynamic_cast<const UnaryExpr *>(root.get());
      return isConstExpr(unary_ptr->child_);
      case ExprType::Aggregate:
        agg_ptr = dynamic_cast<const AggregateExpr *>(root.get());
        return isConstExpr(agg_ptr->child_);
    case ExprType::Binary:
      bin_ptr = dynamic_cast<const BinaryExpr *>(root.get());
      if (bin_ptr->optr_type_ == BinaryExprType::in) {
        // this should be seen as const expr.
        return true;
      }
      return isConstExpr(bin_ptr->left_child_) && 
             isConstExpr(bin_ptr->right_child_);
  }
  throw std::domain_error("(in function isConstExpr)expr type didn't match any?? Impossible!");
}

auto isAggExpr(const AbstractExprRef &root) -> bool {
  cqlAssert(static_cast<bool>(root), "trying to tell if a null expr tree is agg");
  switch (root->GetExprType()) {
    case ExprType::Column: case ExprType::Variable: case ExprType::Const:
      return false;
    case ExprType::Aggregate:
      return true;
    case ExprType::Unary: {
      auto unary_ptr = dynamic_cast<const UnaryExpr *>(root.get());
      return isAggExpr(unary_ptr->child_);
    }
    case ExprType::Binary: {
      auto binary_ptr = dynamic_cast<const BinaryExpr *>(root.get());
      return isAggExpr(binary_ptr->left_child_) || isAggExpr(binary_ptr->right_child_);
    }
  }
  throw std::domain_error("(in function isAggExpr)expr type didn't match any?? Impossible!");
}

void findAggExprs(const AbstractExprRef &root, std::unordered_map<std::string, AbstractExprRef> &exprs) {
  if (!static_cast<bool>(root)) {
    throw std::domain_error("trying to find agg exprs in a null expr tree?? Impossible!");
  }
  switch (root->GetExprType()) {
    case ExprType::Column: case ExprType::Variable: case ExprType::Const:
      return;
    case ExprType::Unary: {
      const UnaryExpr *unary_ptr = dynamic_cast<const UnaryExpr *>(root.get());
      findAggExprs(unary_ptr->child_, exprs);
      return;
    }
    case ExprType::Binary: {
      const BinaryExpr *binary_ptr = dynamic_cast<const BinaryExpr *>(root.get());
      findAggExprs(binary_ptr->left_child_, exprs);
      findAggExprs(binary_ptr->right_child_, exprs);
      return;
    }
    case ExprType::Aggregate: {
      exprs[root->toString()] = root;
      return;
    }
  }
}

auto aggAsColumn(const AbstractExprRef &root) -> AbstractExprRef {
  cqlAssert(static_cast<bool>(root), "argument to aggAsColumn is null");
  switch (root->GetExprType()) {
    case ExprType::Column: case ExprType::Variable: case ExprType::Const:
      return root->Clone();
    case ExprType::Unary: {
      const UnaryExpr *unary_ptr = dynamic_cast<const UnaryExpr *>(root.get());
      return std::make_shared<UnaryExpr>(UnaryExpr(unary_ptr->optr_type_, aggAsColumn(unary_ptr->child_)));
    }
    case ExprType::Binary: {
      const BinaryExpr *binary_ptr = dynamic_cast<const BinaryExpr *>(root.get());
      return std::make_shared<BinaryExpr>(BinaryExpr(binary_ptr->optr_type_, aggAsColumn(binary_ptr->left_child_), 
                                          aggAsColumn(binary_ptr->right_child_)));
    }
    case ExprType::Aggregate: {
      return std::make_shared<ColumnExpr>(ColumnExpr(0, root->toString()));
    }
  }
  throw std::domain_error("(in function aggAsColumn)expr type didn't match any?? Impossible!");
}

}  // namespace cql
