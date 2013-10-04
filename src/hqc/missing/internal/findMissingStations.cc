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

#include "findMissingStations.hh"
#include "getObsPgm.hh"
#include "getStations.hh"
//#include "functors.h"
#include "MissingDataReceiver.hh"
#include <common/KvServiceHelper.hh>
#include <kvalobs/kvcpp/WhichDataHelper.h>
#include <milog/milog.h>
#include <boost/date_time.hpp>

using namespace kvalobs;

namespace FindMissing
{

namespace internal
{

struct expectedAtTime_ {
  template< typename Ignored >
  inline bool operator()( const Ignored &, const kvObsPgm & op ) const
  {
    return op.kl06() or op.kl07();
  }
};

/**
 * \brief Get the list of stations witch should match the given TaskSpecification.
 *
 * \param ts The specification for which to find matching station
 * \param[out] sl The list to be filled with matching station objects.
 */
void matchingStations( const TaskSpecification & ts, ExStationList & sl )
{
  kvalobs::ObsPgm op;

  boost::posix_time::ptime t(ts.to(), boost::posix_time::hours(6));

  // Populate op
  if ( ts.typeID() == 0 ) {
    typedef combined< equal_paramID, expectedAtTime_ > C1;
    typedef single_compare< TaskSpecification, C1 > Cmp;
    C1 ep;
    Cmp func( ts, ep );
    getObsPgmWith<Cmp>( op, t, func );
  }
  else {
    typedef combined< equal_paramID, equal_typeID > C1;
    typedef combined< C1, expectedAtTime_> C2;
    typedef single_compare< TaskSpecification, C2 > Cmp;
    C2 ept;
    Cmp func( ts, ept );
    getObsPgmWith<Cmp>( op, t, func );
  }


  // Populate sl with all stations found in op, which should report at the given time.
  ExStationList all_st;
  extend( getStations(), all_st );
  for ( CIObsPgm it = op.begin(); it != op.end(); ++ it ) {
    if ( it->kl06() or it->kl07() ) {
      CIExStationList s = all_st.find( it->stationID() );
      if ( s != all_st.end() )
        sl.insert( * s );
      else
        sl.insert( it->stationID() ) ;
    }
  }
}

void markCollections(  ExStationList & collected, MissingList & ml, const TaskSpecification & ts )
{
  for ( CIExStationList it = collected.begin(); it != collected.end(); ++ it ) {
    boost::gregorian::date d = std::max( it->klstart.date(), ts.from() );
    for ( ; d < it->klobs.date(); d += boost::gregorian::days(1) ) {
      IMissingList slPtr = ml.find( d );
      if ( slPtr != ml.end() ) {
        IExStationList st = slPtr->second.find( it->stationID() );
        if ( st != slPtr->second.end() ) {
          st->klstart = it->klstart;
          st->klobs = it->klobs;
        }
      }
    }
  }
}

}

}


void findMissingStations( MissingList & ml, const TaskSpecification & ts, const bool & stop )
{
  using namespace FindMissing;
  using namespace FindMissing::internal;

  milog::LogContext context( "findMissingStations" );

  ExStationList possible;
  matchingStations( ts, possible );

  for ( boost::gregorian::date t = ts.from(); t <= ts.to(); t += boost::gregorian::days(1) ) {
  	// Is the thread stopping?
    if ( stop )
      return;

    const boost::posix_time::ptime timeStart( t, boost::posix_time::hours(6) );
    const boost::posix_time::ptime timeStop(  t, boost::posix_time::time_duration(7,0,1) );

    kvservice::WhichDataHelper wdh;
    for ( CIExStationList st = possible.begin(); st != possible.end(); ++ st )
      wdh.addStation( * st, timeStart, timeStop );


    ExStationList & missing = ml[ t ];
    missing = possible;
    ExStationList collected;

    MissingDataReceiver mdr( & missing, & collected, ts, & stop );
    KvServiceHelper::instance()->getKvData(mdr, wdh );

    markCollections( collected, ml, ts );
  }

  const boost::gregorian::date today = boost::gregorian::day_clock::universal_day();
  for ( boost::gregorian::date t = ts.to() + boost::gregorian::days(1); t < today; t += boost::gregorian::days(1) ) {
    if ( stop )
      return;

    const boost::posix_time::ptime timeStart( t, boost::posix_time::hours(6) );
    const boost::posix_time::ptime timeStop(  t, boost::posix_time::hours(7) );
    ExStationList & last = ml[ ts.to() ];
    kvservice::WhichDataHelper wdh;
    for ( CIExStationList st = last.begin(); st != last.end(); ++ st )
      wdh.addStation( * st, timeStart, timeStop );

    ExStationList collected;

    MissingDataReceiver mdr( 0, & collected, ts, & stop );
    KvServiceHelper::instance()->getKvData(mdr, wdh );

    markCollections( collected, ml, ts );
  }
}
