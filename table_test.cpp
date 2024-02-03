#include <fstream>
#include "table.h"

using namespace std;
using namespace cql;

auto main(int argc, char **argv) -> int {
  Table table;
  cout << table.load("test.csv") << endl;
  table.dump(cout);

  return 0;
}