
#include "TypesQueryTask.hh"

#define MILOGGER_CATEGORY "kvhqc.TypesQueryTask"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

bool initMetaType = false;

} // namespace anonymous

TypesQueryTask::TypesQueryTask(size_t priority)
  : QueryTask(priority)
{
  METLIBS_LOG_SCOPE();
  if (not initMetaType) {
    qRegisterMetaType<kvTypes_v>("kvTypes_v");
    initMetaType = true;
  }
}

QString TypesQueryTask::querySql(QString dbversion) const
{
  return "SELECT t.typeid, t.format, t.earlyobs, t.lateobs, t.read,"
      " t.obspgm, t.comment  FROM types t ORDER BY t.typeid";
}

void TypesQueryTask::notifyRow(const ResultRow& row)
{
  METLIBS_LOG_SCOPE();
  int col = 0;

  const int typesid = row.getInt(col++);
  const std::string format = row.getStdString(col++);
  const int earlyobs = row.getInt(col++);
  const int lateobs = row.getInt(col++);
  const std::string read = row.getStdString(col++);
  const std::string obspgm = row.getStdString(col++);
  const std::string comment = row.getStdString(col++);
  
  mTypes.push_back(kvalobs::kvTypes(typesid, format, earlyobs, lateobs,
          read, obspgm, comment));
}

void TypesQueryTask::notifyStatus(int status)
{
  Q_EMIT queryStatus(status);
}
