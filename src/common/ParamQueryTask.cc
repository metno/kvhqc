
#include "ParamQueryTask.hh"

#define MILOGGER_CATEGORY "kvhqc.ParamQueryTask"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

bool initMetaType = false;

} // namespace anonymous

ParamQueryTask::ParamQueryTask(size_t priority)
  : QueryTask(priority)
{
  METLIBS_LOG_SCOPE();
  if (not initMetaType) {
    qRegisterMetaType<hqc::kvParam_v>("kvParam_v");
    initMetaType = true;
  }
}

QString ParamQueryTask::querySql(QString) const
{
  return "SELECT p.paramid, p.name, p.description, p.unit, p.level_scale, p.comment"
      " FROM param p ORDER BY p.paramid";
}

void ParamQueryTask::notifyRow(const ResultRow& row)
{
  METLIBS_LOG_SCOPE();
  int col = 0;

  const int paramid = row.getInt(col++);
  const std::string name = row.getStdString(col++);
  const std::string description = row.getStdString(col++);
  const std::string unit = row.getStdString(col++);
  const int level_scale = row.getInt(col++);
  const std::string comment = row.getStdString(col++);
  mParams.push_back(kvalobs::kvParam(paramid, name, description, unit,
          level_scale, comment));
}
