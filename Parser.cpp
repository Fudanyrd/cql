#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "Parser.h"

namespace cql {

void ParserLog::printTo(std::ostream &os) const {
  /** Header & Table */
  const static std::string header = "=== Parser ===";
  os << header << std::endl;
  switch(exec_type_) {
    case ExecutionType::Select:
      os << "BoundSelect ";
      break;
    case ExecutionType::Update:
      os << "BoundUpdate ";
      break;
    case ExecutionType::Insert:
      os << "BoundInsert ";
      break;
    case ExecutionType::Delete:
      os << "BoundDelete ";
      break;
  }
  os << '{' << std::endl;
  os << "table = " << table_ << std::endl;

  /** Update column */
  os << "update = " << update_column_ << std::endl;

  /** Columns */
  os << "columns = {";
  size_t i = 0;
  if (!columns_.empty()) {
    os << columns_[0]->toString();
  }
  for (i = 1; i < columns_.size(); ++i) {
    os << columns_[i]->toString() << ',';
  }
  os << '}' << std::endl;

  /** Where clause */
  if (static_cast<bool>(where_)) {
    os << "where = " << where_->toString() << std::endl;
  }

  /** Limit & offset */
  if (limit_ != static_cast<size_t>(-1)) {
    os << "limit = " << limit_ << std::endl;
  }
  if (offset_ != 0) {
    os << "offset = " << offset_ << std::endl;
  }

  /** Order bys */
  i = 0;
  if (order_by_.size() != order_by_type_.size()) {
    throw std::domain_error("order by and order by type has differet size?? Impossible!");
  }
  os << "order_bys = [";
  if (!order_by_.empty()) {
    os << "{expr=" << order_by_[i]->toString() << ", type=";
    if (order_by_type_[i] == OrderByType::ASC) {
      os << "ASC";
    } else { os << "DESC"; }
    os << '}';
  }
  for (i = 1; i < order_by_.size(); ++i) {
    os << ',' << "{expr=" << order_by_[i]->toString() << ", type=";
    if (order_by_type_[i] == OrderByType::ASC) {
      os << "ASC";
    } else { os << "DESC"; }
    os << '}';
  }
  os << ']' << std::endl;

  /** End */
  os << '}' << std::endl;
}

auto Parser::Parse(const Command &cmd) -> ParserLog {
  ParserLog res;
  if (cmd.words_[0] == "select") {
    // correct syntax:
    // select {<expr1>, <expr2>, ...} from <table> 
    // (where <clause>) (order by {<expr>, <expr>, ...}) 
    // (limit <const>) (offset <const>)
    res.exec_type_ = ExecutionType::Select;
    return res;
  }

  if (cmd.words_[0] == "update") {
    // correct syntax: update <table> set <col> = <expr>
    res.exec_type_ = ExecutionType::Update;
    res.table_ = cmd.words_[1];
    cqlAssert(cmd.words_[2] == "set", "Invalid update syntax");
    res.update_column_ = cmd.words_[3];
    cqlAssert(cmd.words_[4] == "=", "Invalid update syntax");
    // res.columns_.push_back(std::vector<std::string>(cmd.words_.begin() + 5, cmd.words_.end()));
    res.columns_.push_back(toExprRef(cmd.words_, 5, cmd.words_.size()));
    return res;
  }

  if (cmd.words_[0] == "insert") {
    // correct syntax: insert into <table> values ( <tuple> ), ( <tuple> ), ...
    // and a tuple is: <expr>, <expr>, <expr> ...
    // and expr must NOT include column expressions.
    res.exec_type_ = ExecutionType::Insert;
    cqlAssert(cmd.words_[1] == "into", "Invalid insert syntax");
    res.table_ = cmd.words_[2];
    cqlAssert(cmd.words_[3] == "values", "Invalid insert syntax");
    cqlAssert(cmd.words_[4] == "(", "Invalid insert syntax");
    auto brackets = matchBracket(cmd.words_, {1, '('}, 4, cmd.words_.size());
    // deal with the contents of each bracket.
    for (const auto &bracket : brackets) {
      // split by comma
      auto exprs = splitBy(cmd.words_, {1, ','}, bracket.first, bracket.second);
      for (const auto &expr : exprs) {
        res.columns_.push_back(toExprRef(cmd.words_, expr.first, expr.second));
      }
    }
    return res;
  }

  if (cmd.words_[0] == "delete") {
    // correct syntax: delete from <table> (where clause)
    res.exec_type_ = ExecutionType::Delete;
    cqlAssert(cmd.words_[1] == "from", "Invalid delete syntax at word 1");
    res.table_ = cmd.words_[2];
    if (cmd.words_.size() > 3) {
      // it contains a predicate.
      cqlAssert(cmd.words_[3] == "where", "Invalid delete syntax at word 3");
      cqlAssert(cmd.words_.size() > 4, "Where clause is empty");
      res.where_ = toExprRef(cmd.words_, 4, cmd.words_.size());
    } else {
      res.where_ = nullptr;
    }
    return res;
  }

  // throw an exception.
  std::string err_msg = "unable to recognize execution type at word 0" + cmd.words_[0] + "?? Impossible!";
  throw std::domain_error(err_msg.c_str());
}

}  // namespace cql
