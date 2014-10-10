
#include "RejectedQueryTask.hh"

#include "common/sqlutil.hh"

#include <boost/make_shared.hpp>

namespace /*anonymous*/ {

Time my_qsql_time(const std::string& s)
{
  return timeutil::from_iso_extended_string(s);
}

} // namespace anonymous

RejectedQueryTask::RejectedQueryTask(const TimeSpan& time, size_t priority)
  : QueryTask(priority)
  , mTime(time)
{
}

QString RejectedQueryTask::querySql(QString dbversion) const
{
  std::ostringstream sql;
  sql << "SELECT r.message, r.tbtime, r.decoder, r.comment, r.fixed"
      " FROM rejectdecode r "
      << " WHERE r.tbtime BETWEEN " << time2sql(mTime.t0()) << " AND " << time2sql(mTime.t1())
      << " ORDER BY r.tbtime";
  return QString::fromStdString(sql.str());
}

void RejectedQueryTask::notifyRow(const ResultRow& row)
{
  int col = 0;
  const std::string msg = row.getStdString(col++);
  const timeutil::ptime tbtime = my_qsql_time(row.getStdString(col++));
  const std::string dec = row.getStdString(col++);
  const std::string com = row.getStdString(col++);
  const bool fixed = row.getInt(col++);
  mRejected.push_back(kvalobs::kvRejectdecode(msg, tbtime, dec, com, fixed));
}

void RejectedQueryTask::notifyDone(const QString& withError)
{
  if (not withError.isNull())
    mRejected.clear();
  QueryTask::notifyDone(withError);
}
