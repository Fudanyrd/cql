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
  if (log.table_.empty()) {
    // OK, select <expr1>, <expr2>, ...;
    DataBox box;
    if (!log.columns_.empty()) {
      // box = log.columns_[0]->Evaluate(nullptr, &var_mgn_, 0); 
      // box.printTo(std::cout);
      // pass
    } else {
      throw std::domain_error("no item selected?? Impossible!");
    }
    bool is_const = true;
    for (size_t c = 0; c < log.columns_.size(); ++c) {
      if (!isConstExpr(log.columns_[c])) {
        is_const = false; break;
      }
    }
    if (is_const) {
      DataBox box = log.columns_[0]->Evaluate(nullptr, &var_mgn_, 0);
      box.printTo(std::cout);
      for (size_t c = 1; c < log.columns_.size(); ++c) {
        std::cout << ',';
        box = log.columns_[c]->Evaluate(nullptr, &var_mgn_, 0);
        box.printTo(std::cout);
      }
      std::cout << std::endl;
      return;
    }

    size_t row = 0;  // row number
    bool is_invalid = false;
    while (!is_invalid) {
      box = log.columns_[0]->Evaluate(nullptr, &var_mgn_, row);
      if (box.getType() == TypeId::INVALID) { break; }
      box.printTo(std::cout);
      for (size_t i = 1; i < log.columns_.size(); ++i) {
        std::cout << ',';
        box = log.columns_[i]->Evaluate(nullptr, &var_mgn_, row);
        box.printTo(std::cout);
        if (!is_invalid) { is_invalid = box.getType() == TypeId::INVALID; }
      }

      ++row;
      std::cout << std::endl;
    }
    return;
  }
  // throw std::domain_error("not implemented, please wait...");
  // currently support select <exprs> from <table> where <predicate>.
  AbstractExprRef predicate = static_cast<bool>(log.where_) ? log.where_ :
                              std::make_shared<ConstExpr>(ConstExpr(TypeId::Bool, "True"));
  // log.printTo(std::cout);
  std::ofstream logout("cql.log", std::ios::app);
  log.printTo(logout);
  logout << std::endl;
  logout.close();
  // std::cout << std::endl;
  if (table_mgn_.find(log.table_) == table_mgn_.end()) {
    std::cout << "NOTE: maybe you've forgot to load the table " << log.table_ << '.' << std::endl;
    throw std::domain_error("trying to select from a non-existing table?? Impossible!");
  }
  TableInfo &table_info = table_mgn_[log.table_];
  const std::vector<Tuple> &tuples = table_info.table_ptr_->getTuples();
  if (log.columns_.empty()) {
    throw std::domain_error("no item selected?? Impossible!");
  }
  size_t count = 0;
  size_t max_count = log.limit_ == static_cast<size_t>(-1) ? log.limit_ : log.limit_ + log.offset_;
  for (size_t row = 0; row < tuples.size() && count < max_count; ++row) {
    // deal with tuples one by one.
    if (tuples[row].isDeleted()) { continue; }
    if (!predicate->Evaluate(&(tuples[row]), &var_mgn_, 0).getBoolValue()) { continue; }
    if (count < log.offset_) { ++count; continue; }
    DataBox box = log.columns_[0]->Evaluate(&(tuples[row]), &var_mgn_, 0); 
    box.printTo(std::cout);
    for (size_t col = 1; col < log.columns_.size(); ++col) {
      // hopefully this won't mingle with variable table...
      std::cout << ',';
      box = log.columns_[col]->Evaluate(&(tuples[row]), &var_mgn_, 0); 
      box.printTo(std::cout);
      // append it to a variable(if required)...
    }
    ++count;

    std::cout << std::endl;
  }

  return;
}

}  // namespace cql
