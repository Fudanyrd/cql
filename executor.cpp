#include <algorithm>
#include <unordered_map>

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
      // std::cout << tp.getSchema() << std::endl;
      // for (const auto &box : tp.getData()) {
      //   box.printTo(std::cout); std::cout << ',';
      // } std::cout << std::endl;
      std::vector<DataBox> data;
      for (const auto &expr : columns_) {
        // std::cout << expr->toString() << std::endl;
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

/************************************************
 *               TupleComparator 
 ************************************************/
auto TupleComparator::compare(const Tuple &t1, const Tuple &t2) const -> bool {
  cqlAssert(order_by_.size() == order_by_type_.size(), "size of order_by and order_by_type is incompatible");
  for (size_t i = 0; i < order_by_.size(); ++i) {
    DataBox val1 = order_by_[i]->Evaluate(&t1, nullptr, 0);
    DataBox val2 = order_by_[i]->Evaluate(&t2, nullptr, 0);
    DataBox equal = DataBox::EqualTo(val1, val2);
    if (equal.getBoolValue()) { continue; }
    DataBox less  = DataBox::LessThan(val1, val2);
    return order_by_type_[i] == OrderByType::ASC ? less.getBoolValue() : (!less.getBoolValue());
  }

  return true;
}

/************************************************
 *                SortExecutor 
 ************************************************/
void SortExecutor::Init() {
  count_ = 0;
  child_->Init();
  SortHelper helper(&comparator_, Tuple());

  while (child_->Next(&(helper.tuple_))) {
    helpers_.push_back(helper);
  }

  std::sort(helpers_.begin(), helpers_.end(), SortHelper::compare);
}

auto SortExecutor::Next(Tuple *tuple) -> bool {
  if (count_ >= helpers_.size()) { return false; }
  *tuple = helpers_[count_++].tuple_;
  return true;
}

/************************************************
 *                 AggExecutor 
 ************************************************/
AggExecutor::AggExecutor(const std::vector<AbstractExprRef> &columns, const std::vector<AbstractExprRef> &group_by, 
                         const std::vector<AbstractExprRef> &order_by, AbstractExprRef having, VariableManager *var_mgn, 
                         AbstractExecutorRef child) 
                         : columns_(columns), group_by_(group_by), having_(having), order_by_(order_by) {
  exec_type_ = ExecutorType::AggExec;
  var_mgn_ = var_mgn;
  this->child_ = child;

  /** find aggregation expressions */
  std::unordered_map<std::string, AbstractExprRef> agg_exprs; 
  for (const auto &ref : columns_) {
    findAggExprs(ref, agg_exprs);
  }
  for (const auto &ref : order_by_) {
    findAggExprs(ref, agg_exprs);
  }
  if (static_cast<bool>(having_)) {
    findAggExprs(having_, agg_exprs);
  }  // OK

  /** create schema and then tables */
  Schema table_schema;
  std::vector<AbstractExprRef> agg_vals;  // used to yield data box from tuples.
  agg_vals.reserve(agg_exprs.size());
  // group_by_ : key
  for (const auto &expr : group_by_) {
    // how to set the type of columns??? a potential deficiency...
    table_schema.AppendCol(TypeId::INVALID, expr->toString());
  }
  // agg_exprs: value
  for (const auto &pair : agg_exprs) {
    agg_vals.push_back(pair.second);
    table_schema.AppendCol(TypeId::INVALID, pair.second->toString());
  }
  this->schema_ = table_schema;
  this->table_ = Table(this->schema_); // OK

  /** get tuples from child. */
  std::unordered_map<std::string, std::vector<DataBox>> agg_table;  // table of aggregation values.
  child_->Init();
  Tuple tp;
  while (child_->Next(&tp)) {
    // evaluate the tuple and generate aggregation key.
    std::vector<DataBox> boxes;
    boxes.reserve(group_by_.size() + agg_vals.size());
    std::string key;  // key used to get aggregate values.
    for (const auto &ref : group_by_) {
      DataBox box = ref->Evaluate(&tp, this->var_mgn_, 0);
      key += DataBox::toString(box);
      boxes.push_back(box);
    }
    for (const auto &ref : agg_vals) {
      auto agg_ptr = dynamic_cast<const AggregateExpr *>(ref.get());
      boxes.push_back(static_cast<bool>(agg_ptr) 
                      ? agg_ptr->child_->Evaluate(&tp, this->var_mgn_, 0)
                      : ref->Evaluate(&tp, this->var_mgn_, 0));
    }

    // update the value in aggregation table.
    if (agg_table.find(key) == agg_table.end()) {
      agg_table[key] = boxes;
    } else {
      // update by aggregation type.
      std::vector<DataBox> &data = agg_table[key];
      for (size_t i = group_by_.size(); i < boxes.size(); ++i) {
        AbstractExprRef ref = agg_vals[i - group_by_.size()];
        auto agg_ptr = dynamic_cast<const AggregateExpr *>(ref.get());
        cqlAssert(static_cast<bool>(agg_ptr), "pointer to aggregation expr is null");

        switch(agg_ptr->agg_type_) {
          case AggregateType::Agg: {
            data[i] = boxes[i];
            break;
          }
          case AggregateType::Count: {
            data[i] = DataBox(data[i].getFloatValue() + 1.0);
            break;
          }
          case AggregateType::Max: {
            if (DataBox::LessThan(data[i], boxes[i]).getBoolValue()) {
              data[i] = boxes[i];
            }
            break;
          }
          case AggregateType::Min: {
            if (DataBox::GreaterThan(data[i], boxes[i]).getBoolValue()) {
              data[i] = boxes[i];
            }
            break;
          }
          case AggregateType::Sum: {
            switch(data[i].getType()) {
              case TypeId::Float:
                data[i] = DataBox(data[i].getFloatValue() + DataBox::toFloat(boxes[i]).getFloatValue());
                break;
              case TypeId::Bool: {
                // (bool) a + (bool) b = a | b.
                bool res = data[i].getBoolValue() || DataBox::toBool(boxes[i]).getBoolValue();
                data[i] = DataBox(res); 
                break;
              }
              case TypeId::Char:
                data[i] = DataBox(TypeId::Char, data[i].getStrValue() + DataBox::toString(boxes[i]));
                break;
              default:
                break;
            }
            break;
          }
        }

      }
    }
  }

  for (const auto &pair : agg_table) {
    table_.insertTuple(pair.second);
  }
  // table_.dump(std::cout);
}

}  // namespace cql
