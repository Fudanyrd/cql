#include "variable_manager.h"
#include <iostream>

auto main(int argc, char **argv) -> int {
  cql::VariableManager mg;
  std::vector<cql::DataBox> dat = {0.0, 1.0, 2.0};

  mg.Add("@a", dat);
  dat[1] = {0.0};
  mg.Add("@b", dat);

  for (const auto &d : mg.Retrive("@a")) {
    std::cout << d.getFloatValue() << std::endl;
  }
  for (const auto &d : mg.Retrive("@b")) {
    std::cout << d.getFloatValue() << std::endl;
  }
  return 0;
}