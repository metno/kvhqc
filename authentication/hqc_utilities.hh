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

#ifndef UTILITIES_HH_
#define UTILITIES_HH_

#include <kvalobs/kvData.h>
#include <miconfparser/confsection.h>

#include <QtCore/QString>
#include <QtSql/QSqlDatabase>

#include <string>

namespace Helpers {

void updateCfailed(kvalobs::kvData& data, const std::string& add);

QString typeInfo(int typeID);
QString stationInfo(int stationID);

inline bool is_accumulation(int fd)
{ return fd==2 or fd>=4; }

bool connect2postgres(const QString& qname, const QString& host, const QString& dbname, const QString& user, const QString& password, int port);
bool connect2postgres(const QString& qname, miutil::conf::ConfSection *conf, const std::string& prefix);

} // namespace Helpers

#endif /* UTILITIES_HH_ */
