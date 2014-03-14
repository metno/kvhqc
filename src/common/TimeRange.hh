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

#ifndef COMMON_TIMERANGE_HH
#define COMMON_TIMERANGE_HH 1

#include "util/timeutil.hh"
#include <iosfwd>

class TimeRange {
public:
  TimeRange()
    { }

  TimeRange(const timeutil::ptime& T0, const timeutil::ptime& T1)
    : mT0(T0), mT1(T1) { }
    
  timeutil::ptime& t0()
    { return mT0; }

  const timeutil::ptime& t0() const
    { return mT0; }

  timeutil::ptime& t1()
    { return mT1; }

  const timeutil::ptime& t1() const
    { return mT1; }

  bool contains(const timeutil::ptime& t) const
    { return (mT0.is_not_a_date_time() or mT0 <= t)
          and (mT1.is_not_a_date_time() or t <= mT1); }

  TimeRange intersection(const TimeRange& t) const;

  /** @return true iff both t0 and t1 are undefined */
  bool undef() const;

  /** @return true iff exactly one of t0 and t1 is undefined */
  bool open() const;

  /** @return true iff none of t0 and t1 is undefined, and t0 <= t1 */
  bool closed() const;

  int days() const;
  int hours() const;
  int minutes() const;
  int seconds() const;

  TimeRange extendedByHours(int nHours) const
    { TimeRange t(*this); t.extendByHours(nHours); return t; }

  void extendByHours(int nHours);

  TimeRange shifted(const boost::posix_time::time_duration& s) const
    { TimeRange t(*this); t.shift(s); return t; }

  void shift(const boost::posix_time::time_duration& s);

  bool operator==(const TimeRange& other) const
    { return mT0 == other.mT0 and mT1 == other.mT1; }

  bool operator!=(const TimeRange& other) const
    { return mT0 != other.mT0 or mT1 != other.mT1; }

private:
  timeutil::ptime mT0;
  timeutil::ptime mT1;
};

std::ostream& operator<<(std::ostream& out, const TimeRange& tr);

#endif /* COMMON_TIMERANGE_H */
