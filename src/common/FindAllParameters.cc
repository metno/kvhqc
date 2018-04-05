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

#include "FindAllParameters.hh"

#include "KvMetaDataBuffer.hh"
#include "common/HqcApplication.hh"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

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
    for (const kvalobs::kvParam& p : allParam)
      params.push_back(p.paramID());
  }
  return params;
}

} // namespace Helpers
