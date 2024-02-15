#include "planner.h"
#include "string_util.h"

namespace cql {

auto Planner::GetExecutors(const ParserLog &log) -> AbstractExecutorRef {
  /** Planner should manage a schema for projection executor. */
  static const std::string col_name = "<expr>";
  Schema projection_schema;

  /** Make schema for projection node. */
  for (size_t i = 0; i < log.columns_.size(); ++i) {
    projection_schema.AppendCol(TypeId::INVALID, col_name);
  }

  AbstractExecutorRef res = nullptr;
  /** Sequential scan executor */
  if (!log.table_.empty()) {
    res = std::make_shared<SeqScanExecutor>(SeqScanExecutor(log.table_, table_mgn_));
  }

  /** Filter executor */
  if (static_cast<bool>(log.where_)) {
    res = std::make_shared<FilterExecutor>(FilterExecutor(log.where_, res, var_mgn_));
  }

  /** Limit executor */
  if (log.limit_ != static_cast<size_t>(-1) || log.offset_ != 0) {
    res = std::make_shared<LimitExecutor>(LimitExecutor(log.limit_, log.offset_, res));
  }

  /** Projection executor */
  if (!log.columns_.empty()) {
    res = std::make_shared<ProjectionExecutor>(ProjectionExecutor(&projection_schema, var_mgn_, log.columns_, res));
  }

  /** Destination executor */
  if (!log.destination_.empty()) {
    res = std::make_shared<DestExecutor>(DestExecutor(var_mgn_, log.destination_, res));
  }

  return res;
  /** projection_schema is automatically destroyed... */
}

}  // namespace cql
