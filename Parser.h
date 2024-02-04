#pragma once

#include "Partitioner.h"

namespace cql {

// keywords set of cql language.
const std::vector<std::string> Keywords = {
  "select",   // like select in sql
  "from",
  "limit",
  "offset",
  "where",
  "insert",
  "update",
  "delete",
  "group by",
  "order by",
  "load"   // command for loading a file into memory
  // join is not supported now.
};

}  // namespace cql;