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

  /** Destination */
  if (!destination_.empty()) {
    os << "destination = {" << destination_[0];
    for (size_t i = 1; i < destination_.size(); ++i) {
      os << ',' << destination_[i];
    }
    os << '}' << std::endl;
  }

  /** End */
  os << '}' << std::endl;
}

auto Parser::Parse(const Command &cmd) -> ParserLog {
  ParserLog res;
  if (cmd.words_[0] == "select") {
    // correct syntax:
    // select {<expr1>, <expr2>, ...} (from <table>)
    // (where <clause>) (order by {<expr>, <expr>, ...}) 
    // (limit <const>) (offset <const>) (dest @var1, @var2, ...)
    res.exec_type_ = ExecutionType::Select;
    std::vector<size_t> keyPos = matchKeyword(cmd.words_, keywords);
    for (auto p : keyPos) { std::cout << p << ','; } std::cout << std::endl;
    if (keyPos.size() == 1) {
      // select <expr1>, <expr2>, ...
      auto pairs = splitBy(cmd.words_, ",", 1, cmd.words_.size());
      for (const auto &pair : pairs) {
        res.columns_.push_back(toExprRef(cmd.words_, pair.first, pair.second));
      }
      return res;
    }
    cqlAssert(cmd.words_[keyPos[1]] == "from", "no table selected");

    // TODO(Fudanyrd): deal with columns_.
    auto exprs = splitBy(cmd.words_, ",", 1, keyPos[1]);
    for (const auto &expr : exprs) {
      res.columns_.push_back(toExprRef(cmd.words_, expr.first, expr.second));
    }

    // TODO(Fudanyrd): deal with table.
    res.table_ = cmd.words_[keyPos[1] + 1];

    // TODO(Fudanyrd): deal with where clause.
    size_t pos = 2;  // same position as that in keywords table(defined in Parser.h, line 24)
    if (pos >= keyPos.size()) { return res; }  // caution: index out of bounds.
    if (cmd.words_[keyPos[pos]] == keywords[pos]) {
      // OK, has a where predicate.
      // select ... from ... where predicate <keyword>
      //                     ^^^pos           ^^^ pos+1
      res.where_ = (pos + 1) < keyPos.size() ? toExprRef(cmd.words_, keyPos[pos] + 1, keyPos[pos + 1])
                                             : toExprRef(cmd.words_, keyPos[pos] + 1, cmd.words_.size());
      ++pos;
    }
    if (pos >= keyPos.size()) { return res; }

    // TODO(Fudanyrd): deal with order by.
    pos = 2;
    for (; pos < keyPos.size(); ++pos) {
      if (cmd.words_[keyPos[pos]] == "order by") { break; }
    }
    if (pos != keyPos.size()) {
      size_t left = keyPos[pos] + 1;
      size_t right = pos + 1 < keyPos.size() ? keyPos[pos + 1] : cmd.words_.size();
      std::vector<std::pair<size_t, size_t>> pairs = splitBy(cmd.words_, ",", left, right);
      for (const auto &pair : pairs) {
        if (cmd.words_[pair.second - 1] == "desc") {
          res.order_by_.push_back(toExprRef(cmd.words_, pair.first, pair.second - 1));
          res.order_by_type_.push_back(OrderByType::DESC);
        } else {
          if (cmd.words_[pair.second - 1] == "asc") {
            res.order_by_.push_back(toExprRef(cmd.words_, pair.first, pair.second - 1));
            res.order_by_type_.push_back(OrderByType::ASC);
          } else {
            // NOTE: by default order in ascending order.
            res.order_by_.push_back(toExprRef(cmd.words_, pair.first, pair.second));
            res.order_by_type_.push_back(OrderByType::ASC);
          }
        }
      }
    }

    // TODO(Fudanyrd): deal with limit if exists.
    pos = 2;
    for (; pos < keyPos.size(); ++pos) {
      if (cmd.words_[keyPos[pos]] == "limit") { break; }
    }
    if (pos != keyPos.size()) {
      res.limit_ = static_cast<size_t>(atoi(cmd.words_[keyPos[pos] + 1].c_str()));
    }

    // TODO(Fudanyrd): deal with offset if exists.
    pos = 2;
    for (; pos < keyPos.size(); ++pos) {
      if (cmd.words_[keyPos[pos]] == "offset") { break; }
    }
    if (pos != keyPos.size()) {
      res.offset_ = static_cast<size_t>(atoi(cmd.words_[keyPos[pos] + 1].c_str()));
    }

    // TODO(Fudanyrd): deal with dest.
    pos = 2;
    for (; pos < keyPos.size(); ++pos) {
      if (cmd.words_[keyPos[pos]] == "dest") { break; }
    }
    if (pos != keyPos.size()) {
      for (size_t it = keyPos[pos] + 1; it < cmd.words_.size(); ++it) {
        if (cmd.words_[it] != ",") { res.destination_.push_back(cmd.words_[it]); }
      }
    }

    return res;
  }

  if (cmd.words_[0] == "update") {
    // correct syntax: update <table> set <col> = <expr> (where clause)
    res.exec_type_ = ExecutionType::Update;
    res.table_ = cmd.words_[1];
    cqlAssert(cmd.words_[2] == "set", "Invalid update syntax");
    res.update_column_ = cmd.words_[3];
    cqlAssert(cmd.words_[4] == "=", "Invalid update syntax");
    size_t i;
    for (i = 4; i < cmd.words_.size(); ++i) {
      if (cmd.words_[i] == "where") { break; }
    }
    if (i != cmd.words_.size()) {
      // cmd.words_[i] == "where"
      res.where_ = toExprRef(cmd.words_, i + 1, cmd.words_.size());
      res.columns_.push_back(toExprRef(cmd.words_, 5, i));
      return res;
    }
    res.where_ = nullptr;
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
    cqlAssert(cmd.words_[4] == "{", "Invalid insert syntax(NOTE: tuple in cql begins with \'{\'.)");
    auto brackets = matchBracket(cmd.words_, makeStr(1, '{'), 4, cmd.words_.size());
    // deal with the contents of each bracket.
    for (const auto &bracket : brackets) {
      // split by comma
      auto exprs = splitBy(cmd.words_, makeStr(1, ','), bracket.first + 1, bracket.second);
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
