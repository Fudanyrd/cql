/************************************************
 * string_util.h
 * 
 * Interface of some useful string operations are
 * defined HERE.
 ************************************************/
#pragma once

#include <string>
#include <utility>
#include <vector>

namespace cql {

/**
 * @brief Split a string object on separating character.
 * @return a vector of strings(not including separator)
 */
auto Split(const std::string &str, char ch) -> std::vector<std::string>;

/**
 * @brief Read a file into a string(including '\n'), this is
 * useful when dealing with comments.
 */
auto readToStr(const std::string &filename) -> std::string;

/**
 * If a statement is not true, throw an exception.
 */
void cqlAssert(bool statement, const std::string &err_msg);

/**
 * @brief split a vector of string by separator.
 * @param begin: start search posistion.
 * @param end: end search posisition(NOTE: not included).
 * @return the vector of ranges(a range is {begin, end}).
 */
auto splitBy(const std::vector<std::string> &words, const std::string &separator, 
                   size_t begin, size_t end) -> std::vector<std::pair<size_t, size_t>> ;

/**
 * @brief match the bracket in a list of strings.
 * @param left: form of left bracket(can be any of '(', '{', '['.
 * @param begin: start search posistion.
 * @param end: end search posisition(NOTE: not included).
 * @return the vector of ranges
 * (with range.first = pos of left_bracket, range.right = pos of right_bracket). 
 * @throw an exception if brackets are unable to match.
 */
auto matchBracket(const std::vector<std::string> &words, const std::string &left, size_t begin, size_t end) 
  -> std::vector<std::pair<size_t, size_t>>;

/**
 * @brief judge if a string is end with a character.
 * Ignore character like ' ', '\t', '\n'.
 */
auto endsWith(const std::string &line, char ch) -> bool;

}  // namespace cql