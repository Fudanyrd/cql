/************************************************************************
 * File: Partitioner.h
 * Author: Fudanyrd (email: yangrundong7@gmail.com)
 *
 * Funtion: partition a series of sql sentence 
 * into a list of strings for parser to reason. 
 * Also, characters like ' ', '\n', '\t' are 
 * ignored since parser rarely need these separators.
 *
 * Citation: (on how to implement a partitioner) 
 * http://web.stanford.edu/class/cs143/lectures/lecture03.pdf
 * http://web.stanford.edu/class/cs143/lectures/lecture04.pdf
 **********************************************************************/
#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "string_util.h"

namespace cql {

// store a single cql command.
struct Command {
  Command() = default;
  Command(const Command &c) = default;
  /**
   * @brief clear the command for constructing new objects.
   */
  void Clear() { words_.clear(); }

  /**
   * @brief print the command to output stream
   * For debugging, you generally don't want to use this.
   */
  void printTo(std::ostream &os) const;

  // this doesn't store command separator ';'.
  std::vector<std::string> words_;
};

// you generally just will need one Partitioner
// to partition any number of cql commands.
class Partitioner {
 private:
  // Your helper data structures and helper functions HERE.
  /**
   * @brief convert the caps character to lower.
   */
  static auto tolwr(char ch) -> char {
    return (ch >= 'A' && ch <= 'Z') ? (ch - 'A' + 'a') : ch;
  }

  /**
   * @return true if current character is alphabetic.
   */
  static auto isAlphabet(char ch) -> bool {
    // given that lower case letters are more common...
    return (ch <= 'z' && ch >= 'a') || (ch >= 'A' && ch <= 'Z');
  }

  /**
   * @return true if current character is digits.
   * how to make it support inf? ...
   * i.e. 0-9 and '.'.
   */
  static auto isDigits(char ch) -> bool {
    return (ch >= '0' && ch <= '9') || ch == '.';
  }

 public: 
  /**
   * @brief Initialize a Partitioner.
   */
  Partitioner() = default;

  /**
   * @brief Split a commands into several command.
   * NOTE: like SQL, CQL sentences are separated by ';'.
   */
  static auto partition(const std::string &commands) -> std::vector<Command>;

  /**
   * @brief Do the following changes to cmd:
   * (1) merge '<' '=' and '>' '=' into "<=", ">=".
   * (2) merge 'order'/'group' 'by' into one key word.
   * (3) return false if the command is empty.
   * (4) merge string literal(wrapped by single token).
   * (5) merge variables(begin with '@') and columns(begin with '#') identifier.
   * @return true if current command is not null.
   */
  static auto deepPartition(Command &cmd) -> bool;

  /**
   * For debugging; you generally don't want to call this.
   * @return the number of commands.
   */
  static auto printTo(std::ostream &os, const std::vector<Command> &commands) -> size_t;
};

} // namespace cql