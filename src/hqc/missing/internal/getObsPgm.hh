/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id$

Copyright (C) 2007 met.no

Contact information:
Norwegian Meteorological Institute
Box 43 Blindern
0313 OSLO
NORWAY
email: kvalobs-dev@met.no

This file is part of HQC

HQC is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

HQC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with HQC; if not, write to the Free Software Foundation Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef GETOBSPGM_H_
#define GETOBSPGM_H_

#include <set>
#include <kvalobs/kvObsPgm.h>

namespace kvalobs
{
struct lt_ObsPgm_without_fromtime
{
  bool operator()( const kvalobs::kvObsPgm & a, const kvalobs::kvObsPgm & b )
  {
    if ( a.stationID() != b.stationID() )
      return a.stationID() < b.stationID();
    if ( a.paramID() != b.paramID() )
      return a.paramID() < b.paramID();
    return a.level() < b.level();
  }
};


typedef std::set<kvalobs::kvObsPgm, lt_ObsPgm_without_fromtime> ObsPgm;
typedef ObsPgm::iterator IObsPgm;
typedef ObsPgm::const_iterator CIObsPgm;

/**
 * Get the list of stations in the kvalobs database for which func
 * returns true.
 *
 * \param[out] out The list to fill with the resulting station list.
 * \param func A function to be evaluated for every station in kvalobs.
 *
 * \ingroup group_data_analysis
 */
template<typename Functor>
void getObsPgmWith( ObsPgm & out, const boost::posix_time::ptime & forTime, Functor & func );

}

#include "getObsPgm.tcc"

#endif /*GETOBSPGM_H_*/
