#include "schema.h"

using namespace std;

auto main(int argc, char **argv) -> int {
  string header = "a:float,b:char,c:float,d:char";  
  cql::Schema schema(header);
  schema.printTo(cout);

  while (getline(cin, header)) {
    cql::Schema sch(header);
    sch.printTo(cout);
    std::cout << std::endl;
  }
  return 0;
}