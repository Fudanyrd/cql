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
#include "Parser.h"
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
  AggExec,     // aggregate executor.
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

struct TupleComparator {
  const std::vector<AbstractExprRef> &order_by_;
  const std::vector<OrderByType> &order_by_type_;

  TupleComparator(const std::vector<AbstractExprRef> &order_by, const std::vector<OrderByType> &order_by_type):
    order_by_(order_by), order_by_type_(order_by_type) {}
  
  /** Comparator for tuples */
  auto compare(const Tuple &t1, const Tuple &t2) const -> bool;
};

struct SortHelper {
  /** the tuple comparator to use */
  TupleComparator *cmp_{nullptr};
  /** the tuple it contains */
  Tuple tuple_;

  SortHelper(TupleComparator *cmp, const Tuple &tp): cmp_(cmp), tuple_(tp) {} 
  SortHelper(const SortHelper &that) = default;

  /** Comparator for SortHelper. */
  static auto compare(const SortHelper &h1, const SortHelper &h2) -> bool {
    cqlAssert(h1.cmp_ == h2.cmp_, "two sort helper hold different comparator");
    return h1.cmp_->compare(h1.tuple_, h2.tuple_);
  }
};

class SortExecutor: public AbstractExecutor {
  // const std::vector<AbstractExprRef> &order_by_;
  // const std::vector<OrderByType> &order_by_type_;
 private:
  TupleComparator comparator_;
  /** store the tuples in order */
  std::vector<SortHelper> helpers_;
  /** number of tuples emitted */
  size_t count_{0U};
  /** Get tuples from its child */
  AbstractExecutorRef child_{nullptr};

 public:
  SortExecutor(const std::vector<AbstractExprRef> &order_by, const std::vector<OrderByType> &order_by_type, 
              AbstractExecutorRef child): comparator_(order_by, order_by_type), child_(child) {
    cqlAssert(static_cast<bool>(child_), "child of sort executor is null");
    exec_type_ = ExecutorType::Sort;
  }

  /** return the schema as-is */
  auto GetOutputSchema() const -> const Schema * override { return child_->GetOutputSchema(); }

  /** do the sorting and put the tuples in the vector */
  void Init() override;

  /** emit the tuple one by one */
  auto Next(Tuple *tuple) -> bool override;
};

/**
 * Aggregate executor should calculate the value of 
 * all aggregation expressions, create table of tuples,
 * and send up tables when required. 
 * Then other work can be done by other executors.
 *
 * ie. aggregate executor is (almost certainly) 
 * independent of other executors.
 */
class AggExecutor: public AbstractExecutor {
 private:
  /** find aggregate expr in columns. */
  const std::vector<AbstractExprRef> &columns_;
  /** group bys. */
  const std::vector<AbstractExprRef> &group_by_;
  /** having clause */
  AbstractExprRef having_;
  /** in case aggregate expr is in order by */
  const std::vector<AbstractExprRef> &order_by_;
  /* manually create table of the executor. */
  Table table_;  // not initialized in Init method.
  /** yield tuples from child executor(probably seqscan or filter)*/
  AbstractExecutorRef child_;
  /** Number of tuples emitted. */
  size_t count_{0U};
  /** variable manager?! */
  VariableManager *var_mgn_;

 public:
  AggExecutor(const std::vector<AbstractExprRef> &columns, const std::vector<AbstractExprRef> &group_by, 
              const std::vector<AbstractExprRef> &order_by, AbstractExprRef having, VariableManager *var_mgn,
              AbstractExecutorRef child);

  auto GetOutputSchema() const -> const Schema * override { return table_.getSchema(); }

  void Init() override { count_ = 0U; }

  auto Next(Tuple *tuple) -> bool override {
    std::cout << count_ << std::endl;
    if (count_ >= table_.getNumRows()) {
      return false;
    }
    *tuple = table_.getTuples()[count_++];
    return true;
  }
};

}  // namespace cql
