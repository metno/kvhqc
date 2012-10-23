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
#ifndef __Weather__TimeObs_h__
#define __Weather__TimeObs_h__

#include "timeutil.hh"

#include <kvalobs/kvData.h>

#include <boost/shared_ptr.hpp>

#include <set>
#include <vector>
#include <memory>

namespace boost {
  class thread;
}

namespace Weather
{
  class TimeObs
  {
  public:
    /**
     * \throws std::runtime_error if unable to contact kvalobs.
     */
    TimeObs(int station, const timeutil::ptime& otime, int type);
    ~TimeObs();

    kvalobs::kvData & get( int paramID,
			   const timeutil::ptime& otime);

    void getAll( std::list<kvalobs::kvData *> & out ) const;

    int getStation() const { return station; }
    const timeutil::ptime & getTime() const { return otime; }
    int getType() const { return type; }

  private:

    typedef boost::shared_ptr<kvalobs::kvData> kvDataPtr;

    struct ltKvDataPtr {
      bool operator()( kvDataPtr a, kvDataPtr b ) const;
    };

    typedef std::set<kvDataPtr, ltKvDataPtr> DataCollection;

    DataCollection data;

    int station;
    timeutil::ptime otime;
    int type;
  };

  typedef std::vector<TimeObs> TimeObsList;
  typedef boost::shared_ptr<TimeObsList> TimeObsListPtr;

  /**
   * \warning Caller must delete the returned object when done.
   * \throws std::runtime_error if unable to contact kvalobs.
   */
  TimeObsListPtr getTimeObs( int station,
			     const timeutil::ptime& from,
			     const timeutil::ptime& to,
			     int type,
			     bool processEvents = true );

  /**
   * Starts a new thread, executing getTimeObs, and places the result
   * in holder when done.
   *
   * \warning Caller must delete the holder object when done.
   */
  std::auto_ptr< boost::thread >
  thread_getTimeObs( TimeObsListPtr & holder,
		     int station,
		     const timeutil::ptime& from,
		     const timeutil::ptime& to );
}



#endif // __Weather__TimeObs_h__
