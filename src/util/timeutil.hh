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

#ifndef UTIL_TIMEUTIL_HH
#define UTIL_TIMEUTIL_HH

#include "config.h"
// this is required for making a time-series plot
#include <puTools/miTime.h>

#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <iosfwd>
#include <string>

namespace timeutil {

typedef boost::posix_time::ptime ptime;
typedef boost::gregorian::date pdate;

std::string to_iso_extended_string(const ptime& pt);
std::string to_iso_extended_string(const pdate& pd);

QString to_iso_extended_qstring(const ptime& pt);
QString to_iso_extended_qstring(const pdate& pd);

boost::posix_time::ptime from_iso_extended_string(const std::string& st);

miutil::miTime make_miTime(const ptime& pt);

#ifndef KVALOBS_USE_BOOST_DATE_TIME
inline miutil::miTime to_miTime(const ptime& pt)
{ return make_miTime(pt); }
ptime from_miTime(const miutil::miTime& mt);
inline int day_of_year(const miutil::miTime& mt)
{ return mt.dayOfYear(); }
#else // ! KVALOBS_USE_BOOST_DATE_TIME
inline ptime to_miTime(const ptime& pt) { return pt; }
inline ptime from_miTime(const ptime& pt) { return pt; }
inline int day_of_year(const ptime& pt)
{ return pt.date().day_of_year(); }
#endif // KVALOBS_USE_BOOST_DATE_TIME

ptime from_QDateTime(const QDateTime& qdt);
QDateTime to_QDateTime(const ptime& t);

ptime from_YMDhms(int year, int month, int day, int hour, int minute, int second);

ptime now();

int hourDiff(const ptime& t0, const ptime& t1);

void clearMinutesAndSeconds(QDateTime& dt);
inline QDateTime clearedMinutesAndSeconds(const QDateTime& dt)
{ QDateTime dt0(dt); clearMinutesAndSeconds(dt0); return dt0; }

QDateTime nowWithMinutes0Seconds0();

} // namespace timeutil

std::ostream& operator<<(std::ostream& o, timeutil::ptime const& pt);

#endif /* UTIL_TIMEUTIL_HH */
