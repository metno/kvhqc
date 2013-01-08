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

#define NDEBUG 1
#include "debug.hh"

bool TimeRange::undef() const
{
    return mT0.is_not_a_date_time() or mT1.is_not_a_date_time();
}

int TimeRange::days() const
{
    if( undef() )
        return 0;
    return hours() / 24;
}

// ------------------------------------------------------------------------

int TimeRange::hours() const
{
    if( undef() )
        return 0;
    return (mT1 - mT0).hours();
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
