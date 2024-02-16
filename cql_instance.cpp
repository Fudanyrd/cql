#include <algorithm>
#include <iostream>

#include "cql_instance.h"

namespace cql {

cqlInstance::~cqlInstance() {
  dump();
  for (auto &pair : table_mgn_) {
    if (static_cast<bool>(pair.second.table_ptr_)) {
      delete pair.second.table_ptr_;
    }
  }
}

void cqlInstance::dump() {
  size_t count = 0;
  for (auto &pair : table_mgn_) {
    if (static_cast<bool>(pair.second.table_ptr_)) {
      if (pair.second.is_dirty_) {
        std::ofstream fout((pair.first + ".csv").c_str());
        pair.second.table_ptr_->dump(fout);
        fout.close();
        pair.second.is_dirty_ = false;
        count++;
      }
    }
  }
}

void cqlInstance::run() {
  static const std::string first  = "cql > ";
  static const std::string second = "... > ";
  static const std::string bye = "Bye.";

  std::string line;
  std::string command;

  while (true) {
    std::cout << first;
    if(!std::getline(std::cin, line)) {
      std::cout << bye << std::endl;
      return;
    }

    while (!endsWith(line, ';')) {
      command += line;
      command += " \n";
      std::cout << second;
      if(!std::getline(std::cin, line)) {
        std::cout << bye << std::endl;
        return;
      }
    }
    command += line;
    command += " \n";

    // now execute the command.
    std::vector<Command> cmds; // = Partitioner::partition(line);
    try {
      cmds = Partitioner::partition(command);
    } catch(std::domain_error &e) {
      std::cout << e.what() << std::endl;
      continue;
    }
    for (auto &cmd : cmds) {
      if (Partitioner::deepPartition(cmd)) {
        try {
          execute(cmd);
        } catch (std::domain_error &e) {
          std::cout << e.what() << std::endl;
        }
      }
      // else the line is empty...
      // release the memory used by cmd.
      cmd.Clear();
    }
    cmds.clear();
    command.clear();
  }
}

// execute a complete line(ie. deep partitioned.)
void cqlInstance::execute(const Command &complete) {
  // deal with some special cases.
  if (complete.words_[0] == "read") {
    // read a .sql file.
    std::string filename;
    for (size_t i = 1; i < complete.words_.size(); ++i) {
      filename += complete.words_[i];  // maybe test.sql?
    }
    std::string file = readToStr(filename);
    auto cmds = Partitioner::partition(file);
    for (auto &cmd : cmds) {
      if (Partitioner::deepPartition(cmd)) {
        execute(cmd);
      }
    }
    return;
  }
  // output the schema of a table.
  if (complete.words_[0] == "schema") {
    const std::string &table_ = complete.words_[1];
    if (table_mgn_.find(table_) == table_mgn_.end()) {
      std::cout << "NOTE: maybe you've forgot to load the table " << table_ << '.' << std::endl;
      throw std::domain_error("trying to schema a non-existing table?? Impossible!");
    }
    TableInfo &table_info = table_mgn_[table_];
    const Schema *schema_ptr = table_info.table_ptr_->getSchema();
    schema_ptr->printTo(std::cout);
    std::cout << "OK." << std::endl;
    return;
  }

  // add variable.
  // support both 'var' and 'set' to declare variables.
  if (complete.words_[0] == "set" || complete.words_[0] == "var") {
    if (!complete.words_[1][0] == '@') {
      std::cout << "variable doesn't begin with \'@\'?? Impossible!" << std::endl;
      return;
    } 
    cqlAssert(complete.words_[2] == "=", "invalid variable syntax");
    std::vector<std::pair<size_t, size_t>> exprs = splitBy(complete.words_, ",", 3, complete.words_.size());
    std::vector<DataBox> dat;
    for (const auto &pair : exprs) {
      size_t i = 0;
      AbstractExprRef root = toExprRef(complete.words_, pair.first, pair.second);
      DataBox box = root->Evaluate(nullptr, &var_mgn_, i);
      if (isConstExpr(root)) {
        dat.push_back(box);
        continue;
      }
      while (box.getType() != TypeId::INVALID) {
        dat.push_back(box);
        box = toExprRef(complete.words_, pair.first, pair.second)->Evaluate(nullptr, &var_mgn_, ++i);
      }
    }
    // var_mgn_[complete.words_[1]] = dat;
    var_mgn_.Add(complete.words_[1], dat);

    std::cout << "OK, register variable " << complete.words_[1] << '.' << std::endl;
    return;
  }
  // display the variable list.
  // can also be exprs!
  // syntax: disp/watch @var1 @var2 ...(no commas)
  if (complete.words_[0] == "disp" || complete.words_[0] == "watch") {
    size_t count = 0;
    for (size_t i = 1; i < complete.words_.size(); ++i) {
      if (complete.words_[i][0] != '@') {
        std::cout << "variable doesn't begin with \'@\'?? Impossible!" << std::endl;
      } else {
        std::vector<DataBox> dat;
        try {
          dat = var_mgn_.Retrive(complete.words_[i]);
        } catch (std::domain_error &e) {
          std::cout << "retrieving a non-exist variable " << complete.words_[i] 
                    << "?? Impossible!" << std::endl;
          continue;
        }
        ++count;
        std::cout << complete.words_[i] << " = {";
        if (!dat.empty()) {
          dat[0].printTo(std::cout);
        }
        for (size_t c = 1; c < dat.size(); ++c) {
          std::cout << ',';
          dat[c].printTo(std::cout);
        }
        std::cout << '}' << std::endl;
      }
    }
    std::cout << "OK, displayed " << count << " variables." << std::endl;
    return;
  }

  // load some tables into memory.
  if (complete.words_[0] == "load") {
    size_t i = 1;
    for(; i < complete.words_.size(); ++i) {
      auto ptr = new Table();
      try {
        ptr->load(complete.words_[i] + ".csv");
      } catch (std::domain_error &e) {
        std::cout << "cannot load table " << complete.words_[i] << std::endl;
        continue;
      }
      table_mgn_[complete.words_[i]] = {ptr};
    }

    std::cout << "OK" << std::endl;
    return;
  }

  // cql doesn't support drop.
  // syntax: create table (if not exists) table_name(header).
  if (complete.words_[0] == "create") {
    std::string header;
    std::string name;
    cqlAssert(complete.words_[1] == "table", "create something other than a table");
    auto pairs = matchBracket(complete.words_, "(", 0, complete.words_.size());
    cqlAssert(pairs.size() == 1, "create table with over 2 schemas");
    for (size_t i = pairs[0].first + 1; i < pairs[0].second; ++i) {
      header += complete.words_[i];
    }
    if (complete.words_[2] == "if" && complete.words_[3] == "not" && complete.words_[4] == "exists") {
      // table name = complete.words_[5].
      // create the header.
      name = complete.words_[5];
      auto iter = table_mgn_.find(name);
      if (iter == table_mgn_.end()) {
        auto ptr = new Table(header);
        table_mgn_[name] = {ptr, true};
        // newly created table must be dirty.
      }
    } else {
      std::cout << "WARNING: you may be trying to overwrite a table." << std::endl;
      std::cout << "Hint: use \'CREATE TABLE IF NOT EXISTS\' instead." << std::endl;;
      name = complete.words_[2];
      auto iter = table_mgn_.find(name);
      if (iter != table_mgn_.end()) {
        delete iter->second.table_ptr_;
        iter->second.table_ptr_ = nullptr;
      }
      table_mgn_[name] = {new Table(header), true};
    }

    std::cout << "OK" << std::endl;
    return;
  }

  auto log = Parser::Parse(complete);
  size_t tuples;
  switch(log.exec_type_) {
    case ExecutionType::Insert: {
      tuples = PerformInsert(log);
      std::cout << "OK, " << tuples << " tuple(s) inserted." << std::endl;
      break;
    }
    case ExecutionType::Delete:
     tuples = PerformDelete(log);
      std::cout << "OK, " << tuples << " tuple(s) deleted." << std::endl;
      break;
    case ExecutionType::Update:
      tuples = PerformUpdate(log);
      std::cout << "OK, " << tuples << " tuple(s) updated." << std::endl;
      break;
    case ExecutionType::Select:
      PerformSelect(log);
      std::cout << "OK." << std::endl;
      break;
    default:
      throw std::domain_error("not implemented");
  }
}

/************************************************
 ***          Helper Functions                ***
 ************************************************/
// can help you with sorting the tuples.
std::vector<AbstractExprRef> order_by_;
std::vector<OrderByType> order_by_type_;
class TupleHelper {
 public:
  static auto compare(const Tuple &t1, const Tuple &t2) -> bool {
    for (size_t i = 0; i < order_by_.size(); ++i) {
      DataBox b1 = order_by_[i]->Evaluate(&t1, nullptr, 0);
      DataBox b2 = order_by_[i]->Evaluate(&t2, nullptr, 0);
      if (!DataBox::EqualTo(b1, b2).getBoolValue()) {
        if (order_by_type_[i] == OrderByType::ASC) {
          return DataBox::LessThan(b1, b2).getBoolValue() ? true : false;
        } else {
          return DataBox::LessThan(b1, b2).getBoolValue() ? false : true;
        }
      }
    }

    return true;
  }
};

/**
 * @return number of tuples inserted.
 */
auto cqlInstance::PerformInsert(const ParserLog &log) -> size_t {
  if (table_mgn_.find(log.table_) == table_mgn_.end()) {
    std::cout << "NOTE: maybe you've forgot to load the table " << log.table_ << '.' << std::endl;
    throw std::domain_error("trying to insert into a non-existing table?? Impossible!");
  }
  TableInfo &table_info = table_mgn_[log.table_];
  const Schema *schema_ptr = table_info.table_ptr_->getSchema();
  size_t cols = schema_ptr->getNumCols();
  size_t rows = log.columns_.size();   
  // this can only be checked at runtime.
  cqlAssert(rows % cols == 0, "length of one of the tuple(s) is errorneous");
  rows /= cols;
  size_t count = 0;
  for (size_t i = 0; i < rows; ++i) {
    bool is_const = true;
    for (size_t c = 0; c < cols; ++c) {
      if (is_const) { is_const = isConstExpr(log.columns_[i * cols + c]); }
    }
    if (is_const) {
      // if is const expr, just insert for once.
      std::vector<DataBox> boxes;
      for (size_t c = 0; c < cols; ++c) {
        boxes.push_back(log.columns_[i * cols + c]->Evaluate(nullptr, &var_mgn_, 0));
      }
      ++count;
      table_info.table_ptr_->insertTuple(boxes);
      continue;
    }
    bool has_value = true;
    size_t r = 0;  // size of a variable.
    while (has_value) {
      std::vector<DataBox> boxes;
      DataBox box;
      for (size_t c = 0; c < cols; ++c) {
        box = log.columns_[i * cols + c]->Evaluate(nullptr, &var_mgn_, r);
        boxes.push_back(box);
        if (has_value) { has_value = (box.getType() != TypeId::INVALID); }
      }
      if (has_value) {
        table_info.table_ptr_->insertTuple(boxes);
        ++count;
      }
      ++r;
    }
  }

  // mark as modified.
  table_info.is_dirty_ = true;
  return count;
}
/**
 * @return number of tuples deleted.
 */
auto cqlInstance::PerformDelete(const ParserLog &log) -> size_t {
  if (table_mgn_.find(log.table_) == table_mgn_.end()) {
    std::cout << "NOTE: maybe you've forgot to load the table " << log.table_ << '.' << std::endl;
    throw std::domain_error("trying to delete from a non-existing table?? Impossible!");
  }
  size_t row = 0;    // current row number;
  size_t count = 0;   // number of tuples deleted.
  TableInfo &table_info = table_mgn_[log.table_];
  const std::vector<Tuple> &tuples = table_info.table_ptr_->getTuples();

  for (; row < tuples.size(); ++row) {
    if (static_cast<bool>(log.where_)) {
      DataBox evaluation = log.where_->Evaluate(&(tuples[row]), &var_mgn_, 0);
      if (evaluation.getBoolValue()) {
        if (table_info.table_ptr_->deleteTuple(row)) { ++count; }
      }
    } else {
      if (table_info.table_ptr_->deleteTuple(row)) { ++count; }
    }
  }

  table_info.is_dirty_ = true;
  return count;
}

auto cqlInstance::PerformUpdate(const ParserLog &log) -> size_t {
  if (table_mgn_.find(log.table_) == table_mgn_.end()) {
    std::cout << "NOTE: maybe you've forgot to load the table " << log.table_ << '.' << std::endl;
    throw std::domain_error("trying to update from a non-existing table?? Impossible!");
  }
  size_t row = 0;
  size_t count = 0;
  TableInfo &table_info = table_mgn_[log.table_];
  auto schema_ptr = table_info.table_ptr_->getSchema();
  const std::vector<Tuple> &tuples = table_info.table_ptr_->getTuples();
  // find the column to update.
  size_t col = 0;
  std::string col_name;
  for (size_t i = 1; i < log.update_column_.size(); ++i) {
    col_name.push_back(log.update_column_[i]);
  }
  for (; col < schema_ptr->getNumCols(); ++col) {
    if (col_name == schema_ptr->getColumn(col).second) { break; }
  }
  cqlAssert(col != schema_ptr->getNumCols(), "unable to recognize column name");
  for (; row < tuples.size(); ++row) {
    DataBox box = log.columns_[0]->Evaluate(&(tuples[row]), &var_mgn_, 0);
    if (static_cast<bool>(log.where_)) {
      DataBox pred = log.where_->Evaluate(&(tuples[row]), &var_mgn_, 0);
      if (pred.getBoolValue()) {
        if (table_info.table_ptr_->updateTuple(box, row, col)) { ++count; }
      }
    } else {
      if (table_info.table_ptr_->updateTuple(box, row, col)) { ++count; }
    }
  }

  table_info.is_dirty_ = true;
  return count;
}

void cqlInstance::PerformSelect(const ParserLog &log) {
  /** output to log file*/
  std::ofstream fout("cql.log", std::ios::app);
  log.printTo(fout);
  fout << std::endl;
  fout.close();

  Planner planner(&table_mgn_, &var_mgn_);
  AbstractExecutorRef exec = planner.GetExecutors(log);
  if (!static_cast<bool>(exec)) { return; }
  exec->Init();
  Tuple temp;
  while (exec->Next(&temp)) {
    size_t i = 0;
    temp.getColumnData(i).printTo(std::cout);
    for (i = 1; i < temp.getSize(); ++i) {
      std::cout << ',';
      temp.getColumnData(i).printTo(std::cout);
    }
    std::cout << std::endl;
  }
}

}  // namespace cql
