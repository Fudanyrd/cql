/***********************************************************
 * File: executor.h
 * Author: Fudanyrd (email: yangrundong7@gmail.com)
 *
 * All executors you need to run CQL queries.
 *
 * NOTE:
 * Executors can decrease works done by the CQL 
 * instances and make it easier to change rules of 
 * how CQL Queries are executed.
 *
 * You can also add new executors as you see fit.
 **********************************************************/
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "expr_util.h"
#include "table.h"
#include "tuple.h"
#include "variable_manager.h"

namespace cql {

/** Types of executors. */
enum ExecutorType {
  Dest,        // a special destination executor
               // which pass the tuples as-is and 
               // register at variable table.
  Filter,      // where clause
  Limit,       // limit & offset
  Projection,  // projection execution
  Seqscan,     // load a table.
  Sort,        // sort the tuples of a table.
  Invalid_exec // a executor that does nothing(can be used as default value)
};

/** Base of all executors. */
class AbstractExecutor {
 protected:
  ExecutorType exec_type_{Invalid_exec};        // execution type.
  /** DO NOT CREATE POINTER OF SCHEMA USING new */
  const Schema *output_schema_{nullptr};   // output schema of the executor.
 public:
  AbstractExecutor() = default;
  virtual ~AbstractExecutor() {}

  /**
   * @return the execution type.
   */
  auto GetType() const -> ExecutorType { return exec_type_; }

  /**
   * @return the output schema of the executor.
   */
  virtual auto GetOutputSchema() const -> const Schema * { return output_schema_; }

  /**
   * @param tuple[out] return the next tuple yield by current executor.
   * @return true if has next; false otherwise.
   */
  virtual auto Next(Tuple *tuple) -> bool = 0;

  /**
   * initialize the executor.
   */
  virtual void Init() = 0;
  
};
using AbstractExecutorRef = std::shared_ptr<AbstractExecutor>;

class SeqScanExecutor: public AbstractExecutor {
 public:
  std::string table_name_;            // name of the table to scan into.
 private:
  size_t emitted_{0U};                // record number of tuples emitted.
  /** Table manager passed by planner. */
  std::unordered_map<std::string, TableInfo> *table_mgn_;
  Table *table_ptr_;
 public:
  SeqScanExecutor(const std::string &name, std::unordered_map<std::string, TableInfo> *tb_mgn): 
    table_name_(name), table_mgn_(tb_mgn) {
    this->exec_type_ = ExecutorType::Seqscan;
    auto iter = table_mgn_->find(name);
    cqlAssert(iter != table_mgn_->end(), "cannot find table in checklist");
    table_ptr_ = (iter->second).table_ptr_;
    this->output_schema_ = table_ptr_->getSchema();
  }

  /** Initialize the executor. */
  void Init() override { emitted_ = 0; }

  auto Next(Tuple *tuple) -> bool override;
};

/** NOTE: how to implement its output schema method?? */
class ProjectionExecutor: public AbstractExecutor {
 private:
  VariableManager *var_mgn_;   // variable manager(maybe unused; depends on queries you want to run)
  /** value expression for each column. */
  std::vector<AbstractExprRef> columns_;
  /** Projection executor has exactly none or one child. */
  AbstractExecutorRef child_{nullptr};
  /** Record number of tuples emitted. */
  size_t count_{0U};

 public:
  ProjectionExecutor(const Schema *schema, VariableManager *var_mgn, 
                     const std::vector<AbstractExprRef> &columns, AbstractExecutorRef child):
                       var_mgn_(var_mgn), columns_(columns), child_(child) {
    this->exec_type_ = ExecutorType::Projection;
    this->output_schema_ = schema;
  }

  void Init() override {
    count_ = 0;
    if (static_cast<bool>(child_)) {
      child_->Init();
    }
  }

  auto Next(Tuple *tuple) -> bool override;
};

class DestExecutor: public AbstractExecutor {
 private:
  VariableManager *var_mgn_;
  std::vector<std::string> destinations_;
  /** most likely a projection executor. */
  AbstractExecutorRef child_{nullptr};
 public:
  DestExecutor(VariableManager *var_mgn, const std::vector<std::string> &dests, AbstractExecutorRef child):
    var_mgn_(var_mgn), destinations_(dests), child_(child) {
    // update executor type.
    exec_type_ = ExecutorType::Dest; 
  }

  auto GetOutputSchema() const -> const Schema * override {
    cqlAssert(static_cast<bool>(child_), "child executor of dest executor is null");
    return child_->GetOutputSchema();
  }

  void Init() override {
    cqlAssert(static_cast<bool>(child_), "child executor of dest executor is null");
    child_->Init();
    // A Debatable Implementation:
    // 
    // let's say, if a variable @var
    // already exists and has its value,
    // you may want to clear its value 
    // and then append new ones(ie. overwriting).
    // 
    // If it is the implementation you expected,
    // uncomment the following codes and recompile.
    // code:
    // for (const std::string &var : destinations_) {
    //   var_mgn_->Add(var, {});  // will overwrite original values!
    // }
    // end code
  }

  auto Next(Tuple *tuple) -> bool override;
};

class FilterExecutor: public AbstractExecutor {
 private:
  /** Predicate over tuples(may not be nullptr). */
  AbstractExprRef predicate_;
  /** Child executor where tuples are from. */
  AbstractExecutorRef child_;
  /** Sometimes involve variable manager? */
  VariableManager *var_mgn_;
 public:
  FilterExecutor(AbstractExprRef predicate, AbstractExecutorRef child, VariableManager *var_mgn):
    predicate_(predicate), child_(child), var_mgn_(var_mgn) {
    this->exec_type_ = ExecutorType::Filter;
    cqlAssert(static_cast<bool>(predicate_), "predicate of filter executor is null");
    cqlAssert(static_cast<bool>(child_), "child of filter executor is null");
  }

  /** Filter executor has the same output schema as its child */
  auto GetOutputSchema() const -> const Schema * override {
    return child_->GetOutputSchema();
  }

  void Init() override { child_->Init(); }

  auto Next(Tuple *tuple) -> bool override;
};

class LimitExecutor: public AbstractExecutor {
 private:
  /** Number of tuples emitted */
  size_t count_{0U};
  /** Limit value */
  size_t limit_{static_cast<size_t>(-1)};
  /** offset value */
  size_t offset_{0U};
  /** Child executor to yield tuples. */
  AbstractExecutorRef child_;

 public:
  LimitExecutor(size_t limit, size_t offset, AbstractExecutorRef child):
    limit_(limit), offset_(offset), child_(child) {
    this->exec_type_ = ExecutorType::Limit;
  } 

  auto GetOutputSchema() const -> const Schema * override {
    return child_->GetOutputSchema();
  }

  void Init() override {
    count_ = 0;
    child_->Init();
  }

  auto Next(Tuple *tuple) -> bool override;
};

///////////////////////////////////////////////////////////
// TODO(Fudanyrd): add {limit, sort} executor(s).
///////////////////////////////////////////////////////////

}  // namespace cql
