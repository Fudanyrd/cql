#include <iostream>
#include "expr_util.h"

using namespace std; 

auto main(int argc, char **argv) -> int {
  std::vector<std::string> test = {
    "2", "+", "3", "*", "exp", "(", "1.0", ")"
  };

  cql::AbstractExprRef root = cql::toExprRef(test);
  std::cout << root->toString() << std::endl;
  std::cout << root->Evaluate(nullptr).getFloatValue() << std::endl;

  // input any expressions word by word.
  test.clear();
  std::string temp;
  // if input looks like:
  // exp ( 1 + 2 * 3 ), it can almost be used as a calculator!
  while(cin >> temp) {
    test.push_back(temp);
  }
  root = cql::toExprRef(test);
  std::cout << root->toString() << std::endl;
  std::cout << root->Evaluate(nullptr).getFloatValue() << std::endl;
  // maybe can test on string...
  std::cout << root->Evaluate(nullptr).getStrValue();

  return 0;
}