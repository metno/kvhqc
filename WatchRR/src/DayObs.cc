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
#include "DayObs.h"
#include "enums.h"
#include <puTools/miTime>
#include <kvcpp/KvApp.h>
#include <kvcpp/WhichDataHelper.h>
#include <kvalobs/kvDataOperations.h>
#include <cmath>
#include <limits>
#include <boost/thread.hpp>

#include <boost/date_time/gregorian/gregorian_types.hpp>

#include <iostream>
#include <decodeutility/kvDataFormatter.h>

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace kvservice;

namespace WatchRR
{
  static int interesting[9] = { V4, V4S, V5, V5S, V6, V6S, RR_24, SD, SA };
  static set<int> ilarge( interesting, &interesting[9] );

  static bool sameobs( const kvData &d, int type, int sensor, int level );


  DayObs::DayObs( int station, miDate date, int type, int sensor, int level )
    : station( station )
    , date( date )
    , type( abs(type) )
    , sensor( sensor )
    , level( level )
    , modelRR( std::numeric_limits<float>().min() )
  {
    cerr << "Getting observations for " << date << endl;

    // 7 o'clock , because some stations send RR_24 obs with time = 07:00
    miTime stop( date, miClock( 07, 00, 01 ) );
    date.addDay( -1 );
    miTime start( date, miClock( 11, 59, 59 ) );

    KvObsDataList dataList;

    WhichDataHelper wdh;
    wdh.addStation( station, start, stop );

    bool ok = KvApp::kvApp->getKvData( dataList, wdh );
    if ( not ok )
      throw runtime_error( "Could not get data from kvalobs" );

    for ( KvObsDataList::iterator dl = dataList.begin(); dl != dataList.end(); dl++ )
    {
      for ( KvDataList::const_iterator it = dl->dataList().begin(); it != dl->dataList().end();   it++ )
      {
      	if ( ilarge.find( it->paramID() ) != ilarge.end() and sameobs( *it, type, sensor, level) )
      	{

      	  kvDataPtr d( new kvData( *it ) );

          // Do not overwrite values, unless they are agregated ones.
      	  if ( it->paramID() == RR_24 )
      	  {
      	    DataCollection::const_iterator old = data.find( d );
      	    if ( old != data.end() and (*old)->typeID() > 0 )
      	      continue;
      	  }

      	  data.insert( d );
      	}
      }
    }

    std::list<kvalobs::kvModelData> model;
    ok = KvApp::kvApp->getKvModelData( model, wdh );
    if ( ok ) {
      for ( std::list<kvalobs::kvModelData>::const_iterator it = model.begin(); it != model.end(); ++ it ) {
        if ( it->paramID() == RR_24 ) {
          modelRR = it->original();
          cerr << "Model value at " << it->obstime() << ": " << modelRR << endl;
        }
      }
    }
  }


  DayObs::~DayObs( )
  {
  }

  namespace // Everything here in this NS is related to summer/normal time
  {
	using namespace boost::gregorian;

	// I 1996 ble Norge og alle EU-land enige om at sommertiden starter siste søndag i mars og slutter siste søndag i oktober. (no.wikipedia)

  	// We are hardcoding transitions between summer time and normal time.
  	// Therefore this is only valid for Norway (or possibly the EU), after 1996.

	date summertime_start( int year )
	{
		static last_day_of_the_week_in_month find( Sunday, Mar );
		return find.get_date( year );
	}

	date summertime_end( int year )
	{
		static last_day_of_the_week_in_month find( Sunday, Oct );
		return find.get_date( year );
	}

	bool summertime( const miDate & d )
	{
		date_period summertime( summertime_start( d.year() ), summertime_end( d.year() ) );
	  	date queryDate( d.year(), d.month(), d.day() );
	  	return summertime.contains( queryDate );
  	}
  }

  kvData & DayObs::get( int paramID, const miClock &clock )
  {
    miDate d = date;
    if ( clock > miClock(7,0,0) )
      d.addDay( -1 );
    /*
    if ( type == 402 and clock == miClock( 6,0,0 ) and not summertime( date ) )
    	return get( paramID, miClock( 7,0,0 ) );
    */
    kvDataPtr tmp( new kvData(
      getMissingKvData( station, miTime( d, clock ), paramID, type, sensor, level ) ) );

    pair<DataCollection::iterator, bool> result = data.insert( tmp );
    DataCollection::iterator & dc = result.first;

    return ** dc;
  }

  void DayObs::getAll( std::list<kvalobs::kvData *> & out ) const
  {
    for ( DataCollection::const_iterator it = data.begin(); it != data.end(); ++ it ) {
      out.push_back( it->get() );
    }
  }

  float DayObs::getModelRR() const
  {
    return modelRR;
  }


  bool DayObs::ltKvDataPtr::operator()( kvDataPtr a, kvDataPtr b ) const
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


  static bool sameobs( const kvData &d, int type, int sensor, int level )
  {
    if ( d.paramID() != RR_24 )
      type = abs( type );

    return ( d.typeID() == type )
      and  ( d.sensor() == sensor )
      and  ( d.level() == level );
  }

  DayObsListPtr getDayObs( int station, int type, int sensor, int level,
			   const miDate & from, const miDate & to,
			   bool processEvents )
  {
    DayObsListPtr ret( new DayObsList );
    for ( miDate date = from; date < to; date.addDay() ) {
      ret->push_back( DayObs( station, date, type, sensor, level ) );
    }
    return ret;
  }

  namespace {
    struct thread_obj_getDayObs {
      DayObsListPtr & holder;
      int station, type, sensor, level;
      const miDate & from;
      const miDate & to;

      thread_obj_getDayObs( DayObsListPtr & holder,
			    int station, int type, int sensor, int level,
			    const miDate & from, const miDate & to )
	: holder(holder), station(station), type(type), sensor(sensor)
	, level(level), from(from), to(to)

      {
      }

      void operator()() {
	for ( int i = 0; i < 3; i++ ) {
	  try {
	    holder = getDayObs( station, type, sensor, level, from, to, false );
	    return;
	  }
	  catch ( std::runtime_error & ) {
	    holder = DayObsListPtr( new DayObsList() );
	  }
	}
      }
    };
  }

  auto_ptr< boost::thread >
  thread_getDayObs( DayObsListPtr & holder,
		    int station, int type, int sensor, int level,
		    const miutil::miDate & from, const miutil::miDate & to )
  {
    thread_obj_getDayObs tog( holder, station, type, sensor, level, from, to );
    auto_ptr< boost::thread > ret( new boost::thread( tog ) );
    return ret;
  }
}
