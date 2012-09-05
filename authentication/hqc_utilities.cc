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

void updateCfailed(kvalobs::kvData& data, const std::string& add)
{
    std::string new_cfailed = data.cfailed();
    if( new_cfailed.length() > 0 )
        new_cfailed += ",";
    new_cfailed += add;
    data.cfailed(new_cfailed);
}

miutil::miTime miTimeFromQDateTime(const QDateTime& qdt) {
    const QDate qd = qdt.date();
    const QTime qt = qdt.time();
    return miutil::miTime(qd.year(), qd.month(), qd.day(), qt.hour(), qt.minute(), qt.second());
}