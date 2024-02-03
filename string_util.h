#pragma once

/************************************************
 * string_util.h
 * 
 * Interface of some useful string operations are
 * defined HERE.
 ************************************************/

#include <string>
#include <vector>

namespace cql {

/**
 * @brief Split a string object on separating character.
 * @return a vector of strings(not including separator)
 */
auto Split(const std::string &str, char ch) -> std::vector<std::string>;

}  // namespace cql