/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id: MissingList.cc 3 2007-09-27 10:24:36Z knutj $

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

#include "MissingList.hh"
#include <boost/date_time.hpp>
#include <sstream>

using namespace std;


  ExKvStation::ExKvStation( const kvalobs::kvStation & st )
    : kvalobs::kvStation( st )
{}

ExKvStation::ExKvStation()
    : kvalobs::kvStation( 0, 0, 0, 0, 0, "", 0, 0, "", "", "", 0, 0, boost::posix_time::ptime() )
{}

ExKvStation::ExKvStation( int stationID )
    : kvalobs::kvStation( stationID, 0, 0, 0, 0, "Unknown station", 0, 0, "", "", "", 0, 0, boost::posix_time::ptime() )
{}

ExKvStation::operator int() const
{
  return stationID();
}

void extend( const kvalobs::StationList & from, ExStationList & into )
{
  for ( kvalobs::CIStationList it = from.begin(); it != from.end(); ++ it ) {
    into.insert( * it );
  }
}

void getDateFromStation( DateFromStation & out, const MissingList & ml )
{
  for ( CIMissingList dit = ml.begin(); dit != ml.end(); ++ dit )
    for ( CIExStationList sit = dit->second.begin(); sit != dit->second.end(); ++ sit )
      if ( sit->klstart.is_not_a_date_time() or sit->klobs.is_not_a_date_time() )
        out[ * sit ].push_back( dit->first );
}

string getDateRangeString_( const boost::gregorian::date & from, const boost::gregorian::date & to )
{
  string ret = to_simple_string(from);
  if ( from != to )
    ret += string( " - " ) + to_simple_string(to);
  return ret;
}


std::string stringify( const DateList & dates )
{
  typedef std::vector<boost::gregorian::date>::size_type index;

  if ( dates.empty() )
    return "";

  ostringstream ss;

  boost::gregorian::date current = dates.front();
  index pos = 0;
  for ( index i = 1; i < dates.size(); ++ i ) {
	boost::gregorian::date tmp( current );
	tmp += boost::gregorian::days(i - pos);
    if ( dates[i] != tmp ) {
      ss << getDateRangeString_( current, dates[ i -1 ] ) << ", ";
      current = dates[ i ];
      pos = i;
    }
  }
  ss << getDateRangeString_( current, dates[ dates.size() -1 ] );

  return ss.str();
}
