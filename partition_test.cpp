#include <stdexcept>
#include <vector>

#include "string_util.h"
#include "Partitioner.h"

using namespace std;

auto main(int argc, char **argv) -> int {
  const string filename = "test.sql";  // maybe I should name it test.cql ?!
  string content = cql::readToStr(filename);  // content of the file.

  // real tests begin...
  vector<cql::Command> cmds = cql::Partitioner::partition(content);
  size_t num = cql::Partitioner::printTo(cout, cmds);

  // check if the partitioner generated correct number of cmds.
  std::cout << "found " << num << " command(s)" << std::endl;
  for (cql::Command &cmd : cmds) {
    try {
      cql::Partitioner::deepPartition(cmd);
    } catch (std::domain_error &e) {
      cout << e.what() << endl;
    }
  }
  cql::Partitioner::printTo(cout, cmds);

  return 0;
}