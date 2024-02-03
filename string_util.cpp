#include "string_util.h"

namespace cql {

auto Split(const std::string &str, char ch) -> std::vector<std::string> {
  std::vector<std::string> res;  
  size_t i = 0, j;
  const size_t len = str.size();
  while (i < len) {
    j = i;
    while (j < len && str[j] != ch) { ++j; }
    if (j > i) {
      res.push_back(str.substr(i, j - i));
    }
    i = j + 1;
  }

  return res;
}

}  // namespace cql