#include <fstream>
#include <stdexcept>

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

auto readToStr(const std::string &filename) -> std::string {
  std::ifstream fin(filename.c_str());
  if (!fin.is_open()) {
    throw std::domain_error("cannot open file?? Impossible!");
  }

  std::string str;
  std::string res;
  while (getline(fin, str)) {
    // NOTE: getline don't add the '\n'.
    res += str;
    res += "\n";
  }

  fin.close();
  return res;
}

}  // namespace cql