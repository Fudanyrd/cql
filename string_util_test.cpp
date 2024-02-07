#include <iostream>
#include "string_util.h"

auto main(int argc, char **argv) -> int {
  std::string temp;
  while (std::cin >> temp) {
    for (const auto &str : cql::Split(temp, ' ')) {
      std::cout << str << ',' << str.size();
    }
    std::cout << std::endl;
  }

  std::vector<std::string> test1 = { // pass
    "0", "1", ",", "3,", ",", "5", "6", ",", "8", "9", "10", ","
  };
  for (const auto &pair : cql::splitBy(test1, ",", 0, test1.size())) {
    // std::cout << pair.first << ',' << pair.second << std::endl;
  }

  std::vector<std::string> test2 = {
    "(", "1", "2", ")", "(", "5", "6", "7" , ")", "9", "10"
  };
  for (const auto &pair : cql::matchBracket(test2, "(", 0, test2.size())) {
    std::cout << pair.first << ',' << pair.second << std::endl;
  }

  return 0;
}