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
  {"(", 1},
  // mult, div, power
  {"*", 11},
  {"^", 11},
  {"/", 11},
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
  {"not", 5},
  // and, or, xor
  {"and", 3},
  {"or", 3},
  {"xor", 3},
  // right parentese
  {")", 14}
};
/**
 * @return the in-stack priority of a operator.
 */
auto isp(const std::string &optr) -> int {
  auto iter = isp_table.find(optr);
  // if not match any, probably a unary operator like:
  // sin, cos, tan, ...
  return iter == isp_table.end() ? 13 : iter->second;
}

/************************************************
 * Backgroud knowledge
 * 
 * (2) in-coming priority(icp) of operators:
 * Defined BELOW
 ************************************************/
const std::unordered_map<std::string, int> icp_table = {
  {"(", 14},
  // mult, div, power
  {"*", 10},
  {"^", 10},
  {"/", 10},
  // add, sub
  {"+", 8},
  {"-", 8},
  // comparison
  {"<", 6},
  {">", 6},
  {"<=", 6},
  {">=", 6},
  {"=", 6},
  // not
  {"not", 4},
  // and, or, xor
  {"and", 2},
  {"or", 2},
  {"xor", 2},
  // right parentese
  {")", 1}
};
/**
 * @return the in-coming priority of a operator.
 */
auto icp(const std::string &optr) -> int {
  auto iter = icp_table.find(optr);
  // if not match any, probably a unary operator like:
  // sin, cos, tan, ...
  return iter == isp_table.end() ? 12 : iter->second;
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

/**
 * @return the operator expression of a word.
 * nullptr if failed to match.
 */
auto getOperator(const std::string &word) -> AbstractExprRef {
  if (word.empty()) { return nullptr; }
  // simplify the work...
  if (word[0] == '#') { return nullptr; }

  if (word.size() == 1) {
    // must be one of +-*/~^
    switch (word[0]) {
      case '~': return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Minus, nullptr));
      case '=': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::EqualTo, nullptr, nullptr));
      case '<': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::LessThan, nullptr, nullptr));
      case '>': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::GreaterThan, nullptr, nullptr));
      case '+': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::add, nullptr, nullptr));
      case '-': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::sub, nullptr, nullptr));
      case '*': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::mult, nullptr, nullptr));
      case '/': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::div, nullptr, nullptr));
      case '^': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::power, nullptr, nullptr));
    }
    return nullptr;
  }

  if (word == "<=") {
    return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::LessThanOrEqual, nullptr, nullptr));
  }
  if (word == ">=") {
    return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::GreaterThanOrEqual, nullptr, nullptr));
  }
  if (word == "and") {
    return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::And, nullptr, nullptr));
  }
  if (word == "or") {
    return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::Or, nullptr, nullptr));
  }
  if (word == "xor") {
    return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::Xor, nullptr, nullptr));
  }
  if (word == "not") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Not, nullptr));
  }
  if (word == "exp") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Exp));
  }
  if (word == "ln") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Ln));
  }
  if (word == "sin") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Sin));
  }
  if (word == "cos") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Cos));
  }
  if (word == "tan") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Tan));
  }
  if (word == "sgn") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Sgn));
  }
  if (word == "abs") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Abs));
  }
  if (word == "asin") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Asin));
  }
  if (word == "acos") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Acos));
  }
  if (word == "atan") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Atan));
  }
  if (word == "sqrt") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Sqrt));
  }
  if (word == "sqr") {
    return std::make_shared<UnaryExpr>(UnaryExpr(UnaryExprType::Sqr));
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
  switch(root->GetExprType()) {
    case ExprType::Column: case ExprType::Variable:
      return false;
    case ExprType::Const:
      return true;
    case ExprType::Unary:
      unary_ptr = dynamic_cast<const UnaryExpr *>(root.get());
      return isConstExpr(unary_ptr->child_);
    case ExprType::Binary:
      bin_ptr = dynamic_cast<const BinaryExpr *>(root.get());
      return isConstExpr(bin_ptr->left_child_) && 
             isConstExpr(bin_ptr->right_child_);
  }
}

}  // namespace cql
