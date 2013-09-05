/* -*- c++ -*-
  Kvalobs - Free Quality Control Software for Meteorological Observations

  Copyright (C) 2012 met.no

  Contact information:
  Norwegian Meteorological Institute
  Postboks 43 Blindern
  N-0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of KVALOBS

  KVALOBS is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  KVALOBS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with KVALOBS; if not, write to the Free Software Foundation Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "hqc_utilities.hh"

#include "KvMetaDataBuffer.hh"

#include <miconfparser/confexception.h>
#include <miconfparser/valelement.h>

#include <QtGui/QApplication>
#include <QtSql/QSqlError>

#include <boost/algorithm/string.hpp>

#define MILOGGER_CATEGORY "kvhqc.Helpers" // same as WatchRR2/Helpers.cc!
#include "HqcLogging.hh"

namespace Helpers {

void updateCfailed(kvalobs::kvData& data, const std::string& add)
{
    std::string new_cfailed = data.cfailed();
    if( new_cfailed.length() > 0 )
        new_cfailed += ",";
    new_cfailed += add;
    data.cfailed(new_cfailed);
}

QString typeInfo(int typeID)
{
    try {
        const kvalobs::kvTypes& t = KvMetaDataBuffer::instance()->findType(abs(typeID));

        std::vector<std::string> formats;
        boost::split(formats, t.format(), boost::is_any_of(" ,"));
        if (formats.empty())
            return QString::number(typeID);

        QString info = qApp->translate("Helpers", "%1-station").arg(QString::fromStdString(formats[0]));
        if (typeID < 0)
            info += qApp->translate("Helpers", " generated by kvalobs");

        return info;
    } catch (std::exception&) {
        return QString::number(typeID);
    }
}

QString stationInfo(int stationID)
{
    try {
        const kvalobs::kvStation& s = KvMetaDataBuffer::instance()->findStation(stationID);
        return QString(qApp->translate("Helpers", "%1 %2 %3masl."))
            .arg(stationID).arg(QString::fromStdString(s.name())).arg(s.height());
    } catch (std::exception& e) {
        return QString::number(stationID);
    }
}

bool connect2postgres(const QString& qname, const QString& host, const QString& dbname, const QString& user, const QString& password, int port)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(qname) << LOGVAL(host) << LOGVAL(dbname) << LOGVAL(user) << LOGVAL(password) << LOGVAL(port));

  QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", qname);
  db.setHostName    (host);
  db.setDatabaseName(dbname);
  db.setUserName    (user);
  db.setPassword    (password);
  db.setPort        (port);

  if (not db.open()) {
    HQC_LOG_ERROR("cannot connect to PSQL database: " << db.lastError().text());
    return false;
  } else {
    return true;
  }
}

bool connect2postgres(const QString& qname, miutil::conf::ConfSection *conf, const std::string& prefix)
{
  METLIBS_LOG_SCOPE();
  if (not conf)
    return false;

  using namespace miutil::conf;
  const ValElementList valHost     = conf->getValue(prefix + ".host");
  const ValElementList valDbname   = conf->getValue(prefix + ".dbname");
  const ValElementList valUser     = conf->getValue(prefix + ".user");
  const ValElementList valPassword = conf->getValue(prefix + ".password");
  const ValElementList valPort     = conf->getValue(prefix + ".port");
    
  if (valHost.size() != 1 or valDbname.size() != 1 or valUser.size() != 1 or valPassword.size() != 1 or valPort.size() != 1)
    return false;
    
  try {
    return connect2postgres(qname, QString::fromStdString(valHost    .front().valAsString()),
        QString::fromStdString(valDbname  .front().valAsString()),
        QString::fromStdString(valUser    .front().valAsString()),
        QString::fromStdString(valPassword.front().valAsString()),
        valPort.front().valAsInt());
  } catch (miutil::conf::InvalidTypeEx& e) {
    return false;
  }
}

} // namespace Helpers
