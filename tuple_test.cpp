#include "tuple.h"

using namespace std;  using namespace cql;

auto main(int argc, char **argv) -> int {
  Schema schema("name:char,age:float");
  string temp;
  Tuple tuple(&schema);

  while(getline(cin, temp)) {
    tuple.load(temp);
    tuple.dump(cout);
    std::cout << std::endl;
  }

  return 0;
}