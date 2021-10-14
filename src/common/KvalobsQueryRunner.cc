/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "KvalobsQueryRunner.hh"

#include "HqcApplication.hh"
#include "QueryTask.hh"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

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

  timeutil::ptime getTime(int index) const override { return timeutil::from_QDateTime(mQuery.value(index).toDateTime()); }

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
  int count = 0;
  while (true) {
    if (query.exec(sql)) {
      while (query.next())
        qtask->notifyRow(row);
      return QString();
    }

    const auto& e = query.lastError();
    QString status = e.text();
    count += 1;
    if (count < 3 /*&& e.number() == 40001*/) {
      HQC_LOG_WARN("query '" << sql << "' failed [" << e.number() << "]: " << status << " -- retrying (" << count << ")");
    } else {
      HQC_LOG_ERROR("query '" << sql << "' failed [" << e.number() << "]: " << status);
      return status;
    }
  }
}
