#include "executor.h"

namespace cql {

/************************************************
 *              SeqScanExecutor
 ************************************************/
auto SeqScanExecutor::Next(Tuple *tuple) -> bool {
  const std::vector<Tuple> &tuples = table_ptr_->getTuples();
  if (emitted_ >= tuples.size()) { 
    // no more tuples can be emitted.
    return false;
  }
  *tuple = tuples[emitted_++];
  return true;
}

/************************************************
 *             ProjectionExecutor
 ************************************************/
auto ProjectionExecutor::Next(Tuple *tuple) -> bool {
  if (static_cast<bool>(child_)) {
    // OK, getting values from tables.
    Tuple tp;
    if (child_->Next(&tp)) {
      std::vector<DataBox> data;
      for (const auto &expr : columns_) {
        data.push_back(expr->Evaluate(&tp, var_mgn_, 0));
      }
      *tuple = Tuple(output_schema_, data);
      return true;
    }

    return false;
  }  // end if

  bool isConst = true;
  for (const auto &expr: columns_) {
    if (!isConstExpr(expr)) {
      isConst = false;
    }
  }  // end for

  if (isConst) {
    // OK, the expression is const.
    if (count_ == 0) {
      ++count_;
      std::vector<DataBox> data;
      for (const auto &expr : columns_) {
        data.push_back(expr->Evaluate(nullptr, var_mgn_, 0));
      }
      *tuple = Tuple(output_schema_, data);
      return true;
    }
    return false;
  }

  // if one of the variables has valid value, the iteration continues...
  bool has_next = false;
  std::vector<DataBox> data;
  for (const auto &expr : columns_) {
    DataBox box = expr->Evaluate(nullptr, var_mgn_, count_);
    if (box.getType() != TypeId::INVALID) { has_next = true; }
    data.push_back(box);
  }
  if (!has_next) { return false; }
  // pitfall: forget to update counter.
  ++count_;
  *tuple = Tuple(output_schema_, data);
  return true;
}

/************************************************
 *                DestExecutor
 ************************************************/
auto DestExecutor::Next(Tuple *tuple) -> bool {
  Tuple tp;
  bool has_next = child_->Next(&tp);
  if (!has_next) { return false; }

  // append the value to variables.
  for (size_t i = 0; i < destinations_.size(); ++i) {
    if (destinations_[i] == "@") {
      // empty variable, ignore
      continue;
    }
    // more variables than columns is allowed!
    // though the value will be INVALID in this case...
    var_mgn_->Append(destinations_[i], tp.getColumnData(i));
  }
  *tuple = tp;

  return true;
}

/************************************************
 *               FilterExecutor 
 ************************************************/
auto FilterExecutor::Next(Tuple *tuple) -> bool {
  Tuple tp;
  while (child_->Next(&tp)) {
    DataBox evaluation = predicate_->Evaluate(&tp, var_mgn_, 0);
    if (evaluation.getBoolValue()) {
      *tuple = tp;
      return true;
    }
  }
  return false;
}

/************************************************
 *                LimitExecutor 
 ************************************************/
auto LimitExecutor::Next(Tuple *tuple) -> bool {
  // Tuple tp;
  for (; count_ < offset_; ++count_) {
    if (!child_->Next(tuple)) { return false; }
  }
  size_t end = limit_ == static_cast<size_t>(-1) ? limit_ : (limit_ + offset_);
  if (count_ >= end) { return false; }

  if (!child_->Next(tuple)) {
    return false;
  }
  ++count_;
  return true;
}


}  // namespace cql
