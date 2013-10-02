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

#ifndef MISSINGLIST_H_
#define MISSINGLIST_H_

#include <algorithm>
#include <map>
#include <vector>
#include "StationList.hh"

/**
 * \addtogroup group_data_analysis
 * \{
 */

/**
 * Station information, with added fields for storing data about observation
 * collections.
 *
 * Yes, it's a weird class.
 */
struct ExKvStation : public kvalobs::kvStation
{
  ExKvStation( const kvalobs::kvStation & st );
  ExKvStation();
  ExKvStation( int stationID );

  /**
   * Interpret as an integer, with value of the object's stationID
   */
  operator int() const;

  /**
   * Start time for collection, if any
   */
  mutable boost::posix_time::ptime klstart; // mutable allows editing even when in a std::set

  /**
   * End time for collection, if any
   */
  mutable boost::posix_time::ptime klobs;
};

typedef std::set<ExKvStation, kvalobs::lt_stationID> ExStationList;
typedef ExStationList::iterator IExStationList;
typedef ExStationList::const_iterator CIExStationList;

/**
 * Create a list of ExKvStation from a kvalobs::kvStation list.
 */
void extend( const kvalobs::StationList & from, ExStationList & into );

/**
 * Listing observation information, by obserrvation date
 */
typedef std::map<boost::gregorian::date, ExStationList> MissingList;
typedef MissingList::iterator IMissingList;
typedef MissingList::const_iterator CIMissingList;

typedef std::vector<boost::gregorian::date> DateList;
typedef std::map< ExKvStation, DateList, kvalobs::lt_stationID > DateFromStation;
typedef DateFromStation::iterator IDateFromStation;
typedef DateFromStation::const_iterator CIDateFromStation;

/**
 * Create a list of station->dates from a list of date->stations.
 */
void getDateFromStation( DateFromStation & out, const MissingList & ml );

/**
 * Create a human-readable sting representation of the given time list.
 */
std::string stringify( const DateList & dates );


/**
 * \}
 */


#endif /*MISSINGLIST_H_*/
