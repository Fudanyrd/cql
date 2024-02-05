#include <stdexcept>

#include "Partitioner.h"

namespace cql {

void Command::printTo(std::ostream &os) const {
  if (words_.empty()) {
    // nothing to print!
    return;
  }

  for (const auto &str : words_) {
    os << str << ' ';
  }
  os << std::endl;

  // check the size of each words
  // in case it involved irrelevant character...
  os << words_[0].size();
  for (size_t i = 1; i < words_.size(); ++i) {
    os << ',' << words_[i].size();
  }
  os << std::endl;
}

auto Partitioner::partition(const std::string &commands) -> std::vector<Command> {
  size_t iter = 0U;
  const size_t command_size = commands.size();

  Command cmd;               // temp command.
  std::vector<Command> res;  // result.
  bool is_comment = false;   // whether current character is comment.
  bool is_literal = false;   // whether current character is in a string literal.
  std::string word;

  while (iter < command_size) {
    if (is_literal) {
      // just read all characters into the word.
      std::string word;
      while (iter < command_size && commands[iter] != '\'') {
        word.push_back(commands[iter++]);
      }
      if (iter == command_size) {
        throw std::domain_error("unmatched single quote(reaching the end)? Impossible!");
      }
      cmd.words_.push_back(word);
      cmd.words_.push_back(std::string(1, '\''));
      ++iter;
      is_literal = false;
      continue;
    }
    if (is_comment) {
      // scan util the end of the line; do nothing.
      while (iter < command_size && commands[iter] != '\n') {
        ++iter;
      }
      is_comment = false;
      ++iter;

    } else {
      auto current = commands[iter];
      // if current is in a word:
      if (isAlphabet(current)) {
        // read a full word, and clear the string.
        while (iter < command_size && isAlphabet(commands[iter])) {
          word.push_back(tolwr(commands[iter++]));
        } 
        // register at cmd.
        cmd.words_.push_back(word);
        word.clear();
      } else {
        // it may be digits! eg. 12.5
        if (isDigits(current)) {
          // read a full float number and put into a word.
          while (iter < command_size && isDigits(commands[iter])) {
            word.push_back(tolwr(commands[iter++]));
          } 
          // register at cmd.
          cmd.words_.push_back(word);
          word.clear();
          continue;
        }

        // else it may be '(' ')' ',' "--", '+', '-', '*', '/', ...
        // you have to foresee comment...
        // they are all composed of one character.
        if (current == '\'') {
          // string literal!
          is_literal = true;
          cmd.words_.push_back(std::string(1, current));
          ++iter;
          continue;
        }
        if (current == ';') {
          // end of a cmd.
          res.push_back(cmd);
          cmd.Clear();
          ++iter;
          continue;
        }
        if (current == '-') {
          if (iter + 1 < command_size && commands[iter + 1] == '-') {
            // -- is begin of comment in cql language.
            is_comment = true;
            continue;
          }
        }
        if (current == ' ' || current == '\n' || current == '\t') {
          // encounter separator, continue;
          ++iter;
          continue;
        }
        // for other non-alphabetic characters...
        cmd.words_.push_back(std::string(1, current));
        ++iter;
      }

    }
  }

  return res;
}

auto Partitioner::deepPartition(Command &cmd) -> bool {
  if (cmd.words_.empty()) { return false; }
  std::vector<std::string> new_words;
  size_t numWords = cmd.words_.size();

  for (size_t i = 0; i < numWords; ) {
    if (cmd.words_[i] == "order") {
      if (i + 1 < numWords && cmd.words_[i + 1] == "by") {
        new_words.push_back("order by");
        i += 2; 
        continue;
      }
      // should not execute a buggy query
      cmd.Clear();
      throw std::domain_error("order doesn't follow a keyword by!\n"
      "NOTE: order is a reserved keyword in cql");
    }
    if (cmd.words_[i] == "group") {
      if (i + 1 < numWords && cmd.words_[i + 1] == "by") {
        new_words.push_back("group by");
        i += 2; 
        continue;
      }
      // should not execute a buggy query
      cmd.Clear();
      throw std::domain_error("group doesn't follow a keyword by!\n"
      "NOTE: group is a reserved keyword in cql");
    }
    if (cmd.words_[i] == "<") {
      // maybe it's <= or >=
      if (i + 1 < numWords && cmd.words_[i + 1] == "=") {
        new_words.push_back("<=");
        i+=2;
        continue;
      }
    }
    if (cmd.words_[i] == ">") {
      // maybe it's <= or >=
      if (i + 1 < numWords && cmd.words_[i + 1] == "=") {
        new_words.push_back(">=");
        i+=2;
        continue;
      }
    }
    if (cmd.words_[i] == "#") {
      // definitely a column identifier
      if (i + 1 < numWords) {
        new_words.push_back("#" + cmd.words_[i + 1]);
        i += 2;
        continue;
      }
      throw std::domain_error("@ doesn't follow a column identifier?? Impossible!");
    }
    if (cmd.words_[i] == "@") {
      // definitely a variable identifier.
      if (i + 1 < numWords) {
        new_words.push_back("#" + cmd.words_[i + 1]);
        i += 2;
        continue;
      }
      throw std::domain_error("@ doesn't follow a variable identifier?? Impossible!");
    }

    if (cmd.words_[i] == "\'") {
      // definitely a string literal.
      std::string temp = cmd.words_[i++];
      while (i < numWords && cmd.words_[i] != "\'") {
        temp += cmd.words_[i++];
      }
      if (i == numWords) {
        throw std::domain_error("unmatched character \' used by string literal?? Impossible!");
      }
      temp += cmd.words_[i++];
      new_words.push_back(temp);
      continue;
    }

    // else, it is a common word.
    new_words.push_back(cmd.words_[i++]);
  }

  cmd.words_ = new_words;
  return true;
}

auto Partitioner::printTo(std::ostream &os, const std::vector<Command> &commands) -> size_t {
  for (const auto &cmd : commands) {
    cmd.printTo(os);
  }

  return commands.size(); 
}

}  // namespace cql