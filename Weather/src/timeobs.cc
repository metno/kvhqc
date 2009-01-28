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
#include <miTime>
#include <KvApp.h>
#include <WhichDataHelper.h>
#include <kvDataOperations.h>
#include <cmath>
#include <limits>
#include <boost/thread.hpp>
#include <qmessagebox.h>

#include <iostream>
#include <decodeutility/kvDataFormatter.h>

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace kvservice;

namespace Weather
{
  static int interesting[9] = { V4, V4S, V5, V5S, V6, V6S, RR_24, SD, SA };
  static set<int> ilarge( interesting, &interesting[9] );

  TimeObs::TimeObs( int station, miTime otime, int type )
    : station( station )
    , otime( otime )
  {
    cerr << "Getting observations for " << otime << endl;

    // 7 o'clock , because some stations send RR_24 obs with time = 07:00
    miTime stop( otime );
    stop.addDay();
    miTime start( otime );
    start.addDay(-4);
    KvObsDataList dataList;
    
    WhichDataHelper wdh;
    
    bool stationok =  wdh.addStation( station, start, stop );
    
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

  
  TimeObs::~TimeObs( )
  {
  }

  class NoLateObs /* : public std::exception */ {};

  kvData & TimeObs::get( int paramID, const miTime &otime )
  {
    
    kvDataPtr tmp( new kvData(
			      getMissingKvData( station, otime, paramID, 0, 0, 0 ) ) );
      
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
			   const miTime & from, const miTime & to,
			     int type, bool processEvents )
  {
    TimeObsListPtr ret( new TimeObsList );
    for ( miTime date = from; date <= to; date.addDay() ) {
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
      const miTime & from; 
      const miTime & to;

      thread_obj_getTimeObs( TimeObsListPtr & holder,
			    int station, 
			    const miTime & from, const miTime & to )
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
		    const miutil::miTime & from, const miutil::miTime & to )
  {
    thread_obj_getTimeObs tog( holder, station, from, to );
    auto_ptr< boost::thread > ret( new boost::thread( tog ) );
    return ret;
  }
}
