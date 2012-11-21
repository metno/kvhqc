/* -*- c++ -*-

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
#ifndef __WatchRR__DayObs_h__
#define __WatchRR__DayObs_h__

#include "timeutil.hh"
#include <kvalobs/kvData.h>
#include <boost/shared_ptr.hpp>
#include <set>
#include <vector>
#include <memory>

namespace boost {
  class thread;
}

namespace WatchRR
{
  class DayObs
  {
  public:
    /**
     * \throws std::runtime_error if unable to contact kvalobs.
     */
    DayObs( int station, const timeutil::pdate& date,
	    int type, int sensor, int level );
    ~DayObs( );

    kvalobs::kvData & get( int paramID,
			   const boost::posix_time::time_duration& time = boost::posix_time::time_duration(6,0,0) );

    void getAll( std::list<kvalobs::kvData *> & out ) const;

    /**
     * \return The model value for RR_24, or std::numeric_limits<float>().min()
     * if no model value is available.
     */
    float getModelRR() const;

    int getStation() const { return station; }
    const timeutil::pdate& getDate() const { return date; }
    int getType() const { return type; }
    int getSensor() const { return sensor; }
    int getLevel() const { return level; }

  private:

    typedef boost::shared_ptr<kvalobs::kvData> kvDataPtr;

    struct ltKvDataPtr {
      //bool operator()( const kvDataPtr & a, const kvDataPtr & b ) const;
      bool operator()( kvDataPtr a, kvDataPtr b ) const;
    };

    typedef std::set<kvDataPtr, ltKvDataPtr> DataCollection;

    DataCollection data;

    int station;
    timeutil::pdate date;
    int type;
    int sensor;
    int level;
    float modelRR;
  };

  typedef std::vector<DayObs> DayObsList;
  typedef boost::shared_ptr<DayObsList> DayObsListPtr;

  /**
   * \warning Caller must delete the returned object when done.
   * \throws std::runtime_error if unable to contact kvalobs.
   */
  DayObsListPtr getDayObs( int station, int type, int sensor, int level,
			   const timeutil::pdate & from,
			   const timeutil::pdate & to,
			   bool processEvents = true );

  /**
   * Starts a new thread, executing getDayObs, and places the result
   * in holder when done.
   *
   * \warning Caller must delete the holder object when done.
   */
  std::auto_ptr< boost::thread >
  thread_getDayObs( DayObsListPtr & holder,
		    int station, int type, int sensor, int level,
		    const timeutil::pdate & from, const timeutil::pdate & to );
}



#endif // __WatchRR__DayObs_h__
