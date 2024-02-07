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
  switch(log.exec_type_) {
    case ExecutionType::Insert: {
      size_t tuples = PerformInsert(log);
      std::cout << "OK, " << tuples << " tuple(s) inserted." << std::endl;
      break;
    }
    case ExecutionType::Delete:
    case ExecutionType::Update:
    case ExecutionType::Select:
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
    std::vector<DataBox> boxes;
    for (size_t c = 0; c < cols; ++c) {
      boxes.push_back(log.columns_[count++]->Evaluate(nullptr));
    }
    table_info.table_ptr_->insertTuple(boxes);
  }

  // mark as modified.
  table_info.is_dirty_ = true;
  return rows;
}

}  // namespace cql
