
#include "KvalobsQueryRunner.hh"

#include "HqcApplication.hh"
#include "QueryTask.hh"

#include <QVariant>
#include <QSqlError>
#include <QSqlQuery>

#define MILOGGER_CATEGORY "kvhqc.KvalobsQueryRunner"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

const QString QDBNAME = "kvalobs_bg";
const QString DBVERSION = "d=postgresql:t=kvalobs:v=1";

class QtSqlRow : public ResultRow
{
public:
  QtSqlRow(QSqlQuery& query) : mQuery(query) { }

  bool getBool(int index) const override { return mQuery.value(index).toBool(); }

  int getInt(int index) const override { return mQuery.value(index).toInt(); }

  float getFloat(int index) const override { return mQuery.value(index).toFloat(); }

  QString getQString(int index) const override { return mQuery.value(index).toString(); }

  std::string getStdString(int index) const override { return getQString(index).toStdString(); }

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

QString KvalobsQueryRunner::run(QueryTask* qtask)
{
  METLIBS_LOG_SCOPE();

  QSqlQuery query(mKvalobsDB);
  QtSqlRow row(query);

  const QString sql = qtask->querySql(DBVERSION);
  QString status;
  if (query.exec(sql)) {
    while (query.next())
      qtask->notifyRow(row);
  } else {
    HQC_LOG_ERROR("query '" << sql << "' failed: " << query.lastError().text());
    status = query.lastError().text();
  }
  return status;
}
