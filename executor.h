/*****************************************************
 * File: executor.h
 * Author: Fudanyrd (email: yangrundong7@gmail.com)
 *
 * All executors you need to run CQL queries.
 *****************************************************/
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "expr.h"
#include "tuple.h"
#include "variable_manager.h"

namespace cql {

/** Types of executors. */
enum ExecutorType {
  Assign,      // assignment between variables(ie. @var1 <- @var2)
  Set,         // set the value of variable.
  Seqscan,     // same as print(for variables and columns)
  Projection,
  Invalid
};

/** Base of all executors. */
class AbstractExecutor {
 protected:
  ExecutorType exec_type_{Invalid};        // execution type.
  const Schema *output_schema_{nullptr};   // output schema of the executor.
 public:
  AbstractExecutor() = default;

  /**
   * @return the execution type.
   */
  auto GetType() const -> ExecutorType { return exec_type_; }

  /**
   * @return the output schema of the executor.
   */
  auto GetOutputSchema() const -> const Schema * { return output_schema_; }

  /**
   * @param tuple[out] return the next tuple yield by current executor.
   * @return true if has next; false otherwise.
   */
  auto Next(Tuple *tuple) -> bool = 0;

  /**
   * initialize the executor.
   */
  void Init() = 0;
  
};
using AbstractExecutorRef = std::shared_ptr<AbstractExecutor>;

}  // namespace cql