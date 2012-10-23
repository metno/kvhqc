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
#include "timeobs.h"
#include "enums.h"

#include <kvcpp/KvApp.h>
#include <kvcpp/WhichDataHelper.h>
#include <kvalobs/kvDataOperations.h>
#include <decodeutility/kvDataFormatter.h>

#include <QtGui/qmessagebox.h>

#include <boost/thread.hpp>

#include <cmath>
#include <limits>
#include <iostream>

using namespace std;
using namespace kvalobs;
using namespace kvservice;

namespace Weather
{
  static int interesting[9] = { V4, V4S, V5, V5S, V6, V6S, RR_24, SD, SA };
  static set<int> ilarge( interesting, &interesting[9] );

  TimeObs::TimeObs( int station, const timeutil::ptime& otime, int type )
    : station( station )
    , otime( otime )
  {
    cerr << "Getting observations for " << timeutil::to_iso_extended_string(otime) << endl;

    timeutil::ptime stop( otime );
    stop += boost::gregorian::days(1);
    timeutil::ptime start( otime );
    start += boost::gregorian::days(-4);
    KvObsDataList dataList;

    WhichDataHelper wdh;

    bool stationok =  wdh.addStation( station, timeutil::to_miTime(start), timeutil::to_miTime(stop) );

    bool ok = KvApp::kvApp->getKvData( dataList, wdh );
    if ( not ok )
      throw runtime_error( "Could not get data from kvalobs" );

    for ( KvObsDataList::iterator dl = dataList.begin(); dl != dataList.end(); dl++ ) {
      for ( KvDataList::const_iterator it = dl->dataList().begin(); it != dl->dataList().end();   it++ ) {
      	if ( ilarge.find( it->paramID() ) != ilarge.end() ) {
      	  kvDataPtr d( new kvData( *it ) );
      	  if ( it->paramID() == RR_24 ) {
      	    DataCollection::const_iterator old = data.find( d );
      	    if ( old != data.end() and (*old)->typeID() > 0 )
      	      continue;
      	  }
      	  data.insert( d );
      	}
      }
    }
  }


  TimeObs::~TimeObs()
  {
  }

  class NoLateObs /* : public std::exception */ {};

  kvData & TimeObs::get( int paramID, const timeutil::ptime& otime )
  {

    kvDataPtr tmp( new kvData(getMissingKvData( station, timeutil::to_miTime(otime), paramID, 0, 0, 0 ) ) );

    pair<DataCollection::iterator, bool> result = data.insert( tmp );
    DataCollection::iterator & dc = result.first;

    return ** dc;
  }

  void TimeObs::getAll( std::list<kvalobs::kvData *> & out ) const
  {
    for ( DataCollection::const_iterator it = data.begin(); it != data.end(); ++ it ) {
      out.push_back( it->get() );
    }
  }

  bool TimeObs::ltKvDataPtr::operator()( kvDataPtr a, kvDataPtr b ) const
  {
    int val;

    val = a->stationID() - b->stationID();
    if ( val )
      return val < 0;

    if ( a->obstime() != b->obstime() )
      return a->obstime() < b->obstime();

    val = a->paramID() - b->paramID();
    if ( val )
      return val < 0;

    val = abs(a->typeID()) - abs(b->typeID());
    if ( val )
      return val < 0;

    val = a->sensor() - b->sensor();
    if ( val )
      return val < 0;

    val = a->level() - b->level();
    if ( val )
      return val < 0;

    return false;
  }

  TimeObsListPtr getTimeObs( int station,
			     const timeutil::ptime& from, const timeutil::ptime& to,
			     int type, bool processEvents )
  {
    TimeObsListPtr ret( new TimeObsList );
    for ( timeutil::ptime date = from; date <= to; date += boost::gregorian::days(1) ) {
      //      if ( processEvents )
      //      	KvApp::kvApp->processEvents( 1 );
      ret->push_back( TimeObs( station, date, type ) );
    }
    return ret;
  }

  namespace {
    struct thread_obj_getTimeObs {
      TimeObsListPtr & holder;
      int station, type, sensor, level;
      const timeutil::ptime& from;
      const timeutil::ptime& to;

      thread_obj_getTimeObs( TimeObsListPtr & holder,
			    int station,
			    const timeutil::ptime& from, const timeutil::ptime& to )
	: holder(holder), station(station), from(from), to(to)

      {
      }

      void operator()() {
	for ( int i = 0; i < 3; i++ ) {
	  try {
	    holder = getTimeObs( station, from, to, type, false );
	    return;
	  }
	  catch ( std::runtime_error & ) {
	    holder = TimeObsListPtr( new TimeObsList() );
	  }
	}
      }
    };
  }

  auto_ptr< boost::thread >
  thread_getTimeObs( TimeObsListPtr & holder,
		    int station,
		    const timeutil::ptime& from, const timeutil::ptime& to )
  {
    thread_obj_getTimeObs tog( holder, station, from, to );
    auto_ptr< boost::thread > ret( new boost::thread( tog ) );
    return ret;
  }
}
