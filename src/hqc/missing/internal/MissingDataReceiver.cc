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

#include "MissingDataReceiver.hh"
#include <kvalobs/kvDataOperations.h>
#include <algorithm>
#include <utility>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <milog/milog.h>

#include <iostream>

using namespace std;
using namespace kvalobs;
using namespace kvservice;
using namespace boost;


namespace FindMissing
{

MissingDataReceiver::MissingDataReceiver( ExStationList * missing, ExStationList * collected,
                                          const TaskSpecification & ts, const bool * stop )
    : missing( missing ), collected( collected ), ts( ts )
    , stop( stop ), processKvData_( true ), processTextData_( true )
{}

MissingDataReceiver::~MissingDataReceiver()
{}

bool MissingDataReceiver::next( kvservice::KvObsDataList & odl )
{
  milog::LogContext context( "MissingDataReceiver::next" );

  remove_if_not_missing missing_func( this->missing, this->collected, this->ts );

  for ( IKvObsDataList odl_it = odl.begin(); odl_it != odl.end(); ++ odl_it ) {
    if ( missing ) {
      KvObsData::kvDataList & dl = odl_it->dataList();
      for_each( dl.begin(), dl.end(), missing_func );
    }
    if ( collected ) {
      KvObsData::kvTextDataList & tdl = odl_it->textDataList();
      for_each( tdl.begin(), tdl.end(), missing_func );
    }
  }

  if ( stop != NULL )
    return not * stop;
  return true;
}

MissingDataReceiver::remove_if_not_missing::
remove_if_not_missing( ExStationList * missing, ExStationList * collected, const TaskSpecification & ts )
    : missing( missing ), collected( collected ), ts( ts )
{}

bool MissingDataReceiver::remove_if_not_missing::is_missing( const kvalobs::kvData & data ) const
{
  return (kvalobs::missing( data ) or kvalobs::original_missing( data )) and data.controlinfo().flag(flag::fd) <= 1;
}

void MissingDataReceiver::remove_if_not_missing::operator()( const kvalobs::kvData & data )
{
  if ( missing and data.paramID() == ts.paramID() and ts.isRelevant( data ) )
    if ( not is_missing( data ) )
      missing->erase( data.stationID() );
}

namespace
{
	class endless_iterator
	{
		const tokenizer<offset_separator> & tok_;
		tokenizer<offset_separator>::iterator it_;
		static const std::string empty_;
	public:
		endless_iterator( const tokenizer<offset_separator> & tok ) : tok_( tok ), it_( tok.begin() ) {}

		endless_iterator & operator ++ ()
		{
			if ( it_ != tok_.end() )
				++ it_;
			return * this;
		}
		
		const std::string & operator * () const
		{
			if ( it_ != tok_.end() )
				return * it_;
			return empty_;					
		}
	};
	const std::string endless_iterator::empty_ = "";
}

boost::posix_time::ptime getTime_( const std::string & timeString )
{
  clog << timeString << endl; 
	
  int offsets[] = {4,2,2,2};
  offset_separator f( offsets, offsets + 4 );
  tokenizer<offset_separator> tok( timeString, f );
  
  endless_iterator it( tok );
  
  int year  = boost::lexical_cast<int>( * it );
  int month = boost::lexical_cast<int>( * ++ it );
  int day   = boost::lexical_cast<int>( * ++ it );
  const std::string & hours = * ++ it;
  int hour  = hours.empty() ? 6 : boost::lexical_cast<int>( hours );

  boost::gregorian::date date(year, month, day);
  boost::posix_time::hours clock(hour);

  return boost::posix_time::ptime(date, clock);
}

/*
miutil::miTime getTime_( const miutil::miString & timeString )
{
	clog << timeString << endl; 
	
  int offsets[] = {4,2,2,2};
  offset_separator f( offsets, offsets + 4 );
  tokenizer<offset_separator> tok( timeString, f );
  tokenizer<offset_separator>::iterator it = tok.begin();

  int year  = boost::lexical_cast<int>( * it );
  int month = boost::lexical_cast<int>( * ++ it );
  int day   = boost::lexical_cast<int>( * ++ it );
  int hour  = boost::lexical_cast<int>( * ++ it );
  cerr << timeString << endl;
  // Not always valid - some reports also contain minute and second values:
  //assert( ++ it == tok.end() );

  miutil::miDate date( year, month, day );
  miutil::miClock clock( hour, 0, 0 );
  return miutil::miTime( date, clock );
}
*/

void MissingDataReceiver::remove_if_not_missing::operator()( const kvalobs::kvTextData & textData )
{
  if ( collected and ts.isRelevant( textData ) ) {

    const int KLSTART = 1021;
    const int KLOBS   = 1022;
    const int paramID = textData.paramID();

    if ( paramID != KLSTART and paramID != KLOBS )
      return;

    IExStationList sit = collected->insert( textData.stationID() ).first;

    try {
      if ( paramID == KLSTART )
        sit->klstart = getTime_( textData.original() );
      else
        sit->klobs = getTime_( textData.original() );
    } catch ( boost::bad_lexical_cast & e ) {}
  }
}

}
