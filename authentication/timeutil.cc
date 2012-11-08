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

#include "timeutil.hh"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <memory>

#include "FunctionLogger.hh"

namespace b_pt = boost::posix_time;

namespace timeutil {

std::string to_iso_extended_string(const b_pt::ptime& pt)
{
    std::string ts = b_pt::to_iso_extended_string(pt);
    if( ts.size() > 10 && ts[10] == 'T' )
        ts[10] = ' ';
    return ts;
}

std::string to_iso_extended_string(const pdate& pd)
{
    return (boost::format("%1$04d-%2$02d-%3$02d") % pd.year() % pd.month() % pd.day()).str();
}

b_pt::ptime from_iso_extended_string(const std::string& st)
{
    return b_pt::time_from_string(st);
}

miutil::miTime to_miTime(const b_pt::ptime& pt)
{
    if( pt.is_not_a_date_time() )
        return miutil::miTime();
    const boost::gregorian::date pd = pt.date();
    const boost::posix_time::time_duration pc = pt.time_of_day();
    return miutil::miTime(pd.year(), pd.month(), pd.day(), pc.hours(), pc.minutes(), pc.seconds());
}

b_pt::ptime from_miTime(const miutil::miTime& mt)
{
    if( mt.undef() )
        return ptime();
    else
        return from_YMDhms(mt.year(), mt.month(), mt.day(), mt.hour(), mt.min(), mt.sec());
}

b_pt::ptime from_QDateTime(const QDateTime& qdt) {
    const QDate qd = qdt.date();
    const QTime qt = qdt.time();
    return from_YMDhms(qd.year(), qd.month(), qd.day(), qt.hour(), qt.minute(), qt.second());
}

b_pt::ptime from_YMDhms(int year, int month, int day, int hour, int minute, int second)
{
    //LOG_FUNCTION();
    //std::cerr << "YMD hms = " << year << '-' << month << '-' << day << ' ' << hour << ':' << minute << ':' << second << std::endl;
    try {
        return ptime(boost::gregorian::date(year, month, day), b_pt::time_duration(hour, minute, second));
    } catch(std::exception& e) {
        std::cerr << "YMD hms = " << year << '-' << month << '-' << day << ' ' << hour << ':' << minute << ':' << second << std::endl;
        std::cerr << " => exception: " << e.what() << std::endl;
        throw;
    }
}

b_pt::ptime now()
{
    return b_pt::second_clock::universal_time();
}

int hourDiff(const ptime& t1, const ptime& t0) {
    // TODO this is subtly different from miTime::hourDiff: difference
    // between (18:00, 17:45) is 0 hours here and 1 hour for miTime
    return (t1 - t0).hours();
}

void clearMinutesAndSeconds(QDateTime& dt)
{
    const QTime& t = dt.time();
    dt = dt.addSecs(-60*t.minute() - t.second());
}

QDateTime nowWithMinutes0Seconds0()
{
    QDateTime dt(QDate::currentDate(), QTime::currentTime(), Qt::UTC);
    clearMinutesAndSeconds(dt);
    return dt;
}

} // namespace timeutil

std::ostream& operator<<(std::ostream& o, b_pt::ptime const& pt)
{ o << timeutil::to_iso_extended_string(pt); return o; }
