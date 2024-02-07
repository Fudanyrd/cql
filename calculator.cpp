#include <iostream>

#include "expr_util.h"
#include "Partitioner.h"

// a simple calculator class.
// and a very interesting testing file.
auto main(int argc, char **argv) -> int {
  std::string line;

  while (getline(std::cin, line)) {
    line += ";";   // essential
    std::vector<cql::Command> cmds = cql::Partitioner::partition(line);
    cql::Command cmd = cmds[0];
    cql::Partitioner::deepPartition(cmd);
    cql::AbstractExprRef root; // = cql::toExprRef(cmd.words_);
    cql::DataBox res; //  = root->Evaluate(nullptr);
    try {
      root = cql::toExprRef(cmd.words_);
      res = root->Evaluate(nullptr, nullptr);
    } catch (std::domain_error &e) {
      std::cout << e.what() << std::endl;
      continue;
    }

    if (res.getType() == cql::TypeId::Char) {
      std::cout << res.getStrValue() << std::endl;
    } else {
      res.printTo(std::cout);
      std::cout << std::endl;
    }

  }

  std::cout << "bye." << std::endl;
  return 0;
}