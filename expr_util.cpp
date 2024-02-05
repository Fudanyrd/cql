#include <stack>

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

/**
 * @return the in-stack priority of a operator.
 */
auto isp(const std::string &optr) -> int {
  if (optr == "(") {
    return 1;
  }
  if (optr == "*" || optr == "/") {
    return 5;
  }
  if (optr == "+" || optr == "-") {
    return 3;
  }
  if (optr == ")") {
    return 8;
  }
  // if not match any, probably a unary operator like:
  // sin, cos, tan, ...
  return 7;
}

/************************************************
 * Backgroud knowledge
 * 
 * (2) in-coming priority(icp) of operators:
 * Defined BELOW
 ************************************************/

/**
 * @return the in-coming priority of a operator.
 */
auto icp(const std::string &optr) -> int {
  if (optr == "(") {
    return 8;
  }
  if (optr == "*" || optr == "/") {
    return 4;
  }
  if (optr == "+" || optr == "-") {
    return 2;
  }
  if (optr == ")") {
    return 1;
  }
  // if not match any, probably something like:
  // sin, cos, tan, ...
  return 6;
}

/**
 * @return not nullptr if the word is a constant expression.
 */
auto getConstExpr(const std::string &word) -> AbstractExprRef {
  if (word.empty()) {
    return nullptr;
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
  return std::make_shared<ColumnExpr>(ColumnExpr(0U, word));
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
      case '+': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::add, nullptr, nullptr));
      case '-': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::sub, nullptr, nullptr));
      case '*': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::mult, nullptr, nullptr));
      case '/': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::div, nullptr, nullptr));
      case '^': return std::make_shared<BinaryExpr>(BinaryExpr(BinaryExprType::power, nullptr, nullptr));
    }
    return nullptr;
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

}  // namespace cql
