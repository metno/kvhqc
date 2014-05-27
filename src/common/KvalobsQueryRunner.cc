
#include "KvalobsQueryRunner.hh"

#include "HqcApplication.hh"
#include "QueryTask.hh"

#include <QtCore/QVariant>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#define MILOGGER_CATEGORY "kvhqc.KvalobsQueryRunner"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

const QString QDBNAME = "kvalobs_bg";
const QString DBVERSION = "kvalobs:1";

class QtSqlRow : public ResultRow
{
public:
  QtSqlRow(QSqlQuery& query) : mQuery(query) { }

  int getInt(int index) const
    { return mQuery.value(index).toInt(); }

  float getFloat(int index) const
    { return mQuery.value(index).toFloat(); }
  
  QString getQString(int index) const
    { return mQuery.value(index).toString(); }

  std::string getStdString(int index) const
    { return getQString(index).toStdString(); }

private:
  QSqlQuery& mQuery;
};

} // namespace anonymous

// ========================================================================

void KvalobsQueryRunner::initialize()
{
  METLIBS_LOG_SCOPE();
  mKvalobsDB = hqcApp->kvalobsDB(QDBNAME);
}

// ------------------------------------------------------------------------

void KvalobsQueryRunner::finalize()
{
  METLIBS_LOG_SCOPE();
  mKvalobsDB.close();
  QSqlDatabase::removeDatabase(QDBNAME);
}

// ------------------------------------------------------------------------

void KvalobsQueryRunner::run(QueryTask* qtask)
{
  METLIBS_LOG_SCOPE();

  QSqlQuery query(mKvalobsDB);
  QtSqlRow row(query);

  qtask->notifyStatus(QueryTask::STARTED);

  const QString sql = qtask->querySql(DBVERSION);
  if (query.exec(sql)) {
    while (query.next())
      qtask->notifyRow(row);
    qtask->notifyStatus(QueryTask::COMPLETE);
  } else {
    HQC_LOG_ERROR("query '" << sql << "' failed: " << query.lastError().text());
    qtask->notifyStatus(QueryTask::FAILED);
    qtask->notifyError(query.lastError().text());
  }
}
