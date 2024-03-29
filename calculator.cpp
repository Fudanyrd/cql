#include <fstream>
#include <iostream>

#include "expr_util.h"
#include "Partitioner.h"

// a simple calculator class.
// and a very interesting testing file.
auto main(int argc, char **argv) -> int {
  std::ofstream fout("calculator.log", std::ios::app);
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
      fout << root->toString() << std::endl;
      res = root->Evaluate(nullptr, nullptr, 0);
    } catch (std::domain_error &e) {
      std::cout << e.what() << std::endl;
      fout << e.what() << std::endl;
      continue;
    }

    if (res.getType() == cql::TypeId::Char) {
      std::cout << res.getStrValue() << std::endl;
    } else {
      res.printTo(std::cout);
      res.printTo(fout);
      std::cout << std::endl;
      fout << std::endl;
    }

  }

  std::cout << "bye." << std::endl;
  return 0;
}
