
#include "FindAllParameters.hh"

#include "HqcApplication.hh"

#include <QtCore/QVariant>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <sstream>

#define MILOGGER_CATEGORY "kvhqc.FindAllParameters"
#include "HqcLogging.hh"

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
    METLIBS_LOG_ERROR("search for parameters failed: " << query.lastError().text());
  }
  return params;
}

} // namespace Helpers
