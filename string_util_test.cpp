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
  return 0;
}