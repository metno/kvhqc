/* -*- c++ -*-
   Kvalobs - Free Quality Control Software for Meteorological Observations

   Copyright (C) 2011-2012 met.no

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

#include "TimeRange.hh"

#include <ostream>

#define MILOGGER_CATEGORY "kvhqc.TimeRange"
#include "util/HqcLogging.hh"

bool TimeRange::undef() const
{
  return (mT0.is_not_a_date_time() and mT1.is_not_a_date_time());
}

bool TimeRange::open() const
{
  return (mT0.is_not_a_date_time() xor mT1.is_not_a_date_time());
}

bool TimeRange::closed() const
{
  return not (mT0.is_not_a_date_time() or mT1.is_not_a_date_time()) and mT0 <= mT1;
}

TimeRange TimeRange::intersection(const TimeRange& t) const
{
  timeutil::ptime t0, t1;
  if (mT0.is_not_a_date_time())
    t0 = t.mT0;
  else if (t.mT0.is_not_a_date_time())
    t0 = mT0;
  else
    t0 = std::max(mT0, t.mT0);

  if (mT1.is_not_a_date_time())
    t1 = t.mT1;
  else if (t.mT1.is_not_a_date_time())
    t1 = mT1;
  else
    t1 = std::min(mT1, t.mT1);

  if (t0 <= t1)
    return TimeRange(t0, t1);
  else
    return TimeRange();
}

// ------------------------------------------------------------------------

int TimeRange::days() const
{
  return hours() / 24;
}

// ------------------------------------------------------------------------

int TimeRange::hours() const
{
  return minutes() / 60;
}

// ------------------------------------------------------------------------

int TimeRange::minutes() const
{
  return seconds() / 60;
}

// ------------------------------------------------------------------------

int TimeRange::seconds() const
{
  if (undef())
    return 0;
  return (mT1 - mT0).total_seconds();
}

// ------------------------------------------------------------------------

void TimeRange::extendByHours(int nHours)
{
  const boost::posix_time::time_duration h = boost::posix_time::hours(nHours);
  mT0 -= h;
  mT1 += h;
}

// ------------------------------------------------------------------------

void TimeRange::shift(const boost::posix_time::time_duration& s)
{
  mT0 += s;
  mT1 += s;
}

// ========================================================================

std::ostream& operator<<(std::ostream& out, const TimeRange& tr)
{
  out << '[' << tr.t0() << "--" << tr.t1() << ']';
  return out;
}
