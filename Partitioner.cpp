#include "Partitioner.h"

namespace cql {

void Command::printTo(std::ostream &os) const {
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
  std::string word;

  while (iter < command_size) {
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

auto Partitioner::printTo(std::ostream &os, const std::vector<Command> &commands) -> size_t {
  for (const auto &cmd : commands) {
    cmd.printTo(os);
  }

  return commands.size(); 
}

}  // namespace cql