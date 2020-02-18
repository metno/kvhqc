
#include "TextDataQueryTask.hh"

#include "common/sqlutil.hh"

namespace /*anonymous*/ {

Time my_qsql_time(const std::string& s)
{
  return timeutil::from_iso_extended_string(s);
}

} // namespace anonymous

TextDataQueryTask::TextDataQueryTask(const int stationId, const TimeSpan& time, size_t priority)
  : QueryTask(priority)
  , mStationId(stationId)
  , mTime(time)
{
}

QString TextDataQueryTask::querySql(QString) const
{
  std::ostringstream sql;
  sql << "SELECT td.obstime, td.original, td.paramid, td.tbtime, td.typeid"
      " FROM text_data td WHERE "
      " td.stationid = " << mStationId
      << " AND td.obstime BETWEEN " << time2sql(mTime.t0()) << " AND " << time2sql(mTime.t1())
      << " ORDER BY td.tbtime";
  return QString::fromStdString(sql.str());
}

void TextDataQueryTask::notifyRow(const ResultRow& row)
{
  TxtDat txtd;
  txtd.stationId = mStationId;

  int col = 0;
  txtd.obstime   = my_qsql_time(row.getStdString(col++));
  txtd.original  = row.getStdString(col++);
  txtd.paramId   = row.getInt(col++);
  txtd.tbtime    = my_qsql_time(row.getStdString(col++));
  txtd.typeId    = row.getInt(col++);
  mTextData.push_back(txtd);
}

void TextDataQueryTask::notifyDone(const QString& withError)
{
  if (not withError.isNull())
    mTextData.clear();
  QueryTask::notifyDone(withError);
}
