#include "cql_instance.h"

auto main(int argc, char **argv) -> int {
  cql::cqlInstance instance;
  instance.run();
  // instance.dump();
  return 0;
}