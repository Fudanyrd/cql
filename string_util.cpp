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

void cqlAssert(bool statement, const std::string &err_msg) {
  static const std::string tail = "?? Impossible!";
  if (!statement) {
    throw std::domain_error((err_msg + tail).c_str());
  }
}

auto splitBy(const std::vector<std::string> &words, const std::string &separator, 
                   size_t begin, size_t end) -> std::vector<std::pair<size_t, size_t>> {
  // result vector.
  std::vector<std::pair<size_t, size_t>> res;
  size_t i = begin, j;
  while (i < end) {
    j = i;
    while (j < end && words[j] != separator) { ++j; }
    res.push_back({i, j});
    i = j + 1;
  }
  if (i < end) {
    res.push_back({i, end});
  }
  return res;
}

auto matchBracket(const std::vector<std::string> &words, const std::string &left, size_t begin, size_t end) 
  -> std::vector<std::pair<size_t, size_t>> {
  std::vector<std::pair<size_t, size_t>> res;
  cqlAssert(left.size() == 1, "# of character of left is not 1");
  const std::string left_bracket = left;
  std::string right_bracket;
  switch(left[0]) {
    case '(': right_bracket = ")"; break;
    case '[': right_bracket = "]"; break;
    case '{': right_bracket = "}"; break;
    default:
      throw std::domain_error("invalid argument to left?? Impossible!");
  }

  size_t l = begin, r;
  while (l < end) {
    if (words[l] == right_bracket) {
      throw std::domain_error("unable to match left bracket?? Impossible!");
    }
    if (words[l] != left_bracket) {
      ++l; continue;
    }
    // find the position of l.
    // try to match r...
    r = l + 1;
    while (r < end && words[r] != right_bracket) { ++r; }
    if (r == end) {
      throw std::domain_error("unable to match right bracket?? Impossible!");
    }
    // find one matched pair...
    res.push_back({l, r});
    l = r + 1;
  }

  return res;
}

auto endsWith(const std::string &line, char ch) -> bool {
  if (line.empty()) { return false; }
  for(size_t i = line.size() - 1; i >= 0; --i) {
    if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n') {
      return line[i] == ch;
    }
  }
  return false;
}

}  // namespace cql