
#include "FindAllParameters.hh"

#include "KvMetaDataBuffer.hh"
#include "common/HqcApplication.hh"

#include <QVariant>
#include <QSqlError>
#include <QSqlQuery>

#include <boost/foreach.hpp>

#include <sstream>

#define MILOGGER_CATEGORY "kvhqc.FindAllParameters"
#include "util/HqcLogging.hh"

namespace Helpers {

std::vector<int> findAllParameters(bool historic)
{
  METLIBS_LOG_SCOPE();

  std::ostringstream sql;
  sql << "SELECT DISTINCT paramid FROM obs_pgm";
  if (not historic)
    sql << " WHERE totime IS NULL";
  sql << " ORDER BY paramid";
  METLIBS_LOG_DEBUG(LOGVAL(sql.str()));

  QSqlQuery query(hqcApp->kvalobsDB());
  std::vector<int> params;
  if (query.exec(QString::fromStdString(sql.str()))) {
    while (query.next()) {
      const int paramId = query.value(0).toInt();
      params.push_back(paramId);
    }
  } else {
    HQC_LOG_ERROR("failed to fetch parameter list from kvalobs SQL db -- using kvParam; error was: " << query.lastError().text());

    const std::vector<kvalobs::kvParam> allParam = KvMetaDataBuffer::instance()->allParams();
    BOOST_FOREACH(const kvalobs::kvParam& p, allParam)
        params.push_back(p.paramID());
  }
  return params;
}

} // namespace Helpers
