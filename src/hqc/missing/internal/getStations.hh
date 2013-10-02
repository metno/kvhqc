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

#ifndef GETSTATIONS_H_
#define GETSTATIONS_H_

#include "StationList.hh"
#include <list>
#include <kvalobs/kvStation.h>

namespace kvalobs
{
	/**
	 * Get the station list from kvalobs.
	 *  
	 * The result will be cached, so only the first call to this method will cause 
	 * kvalobs to be contacted.
	 *
	 * \ingroup group_data_analysis
	 */
	inline const StationList & getStations();

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
	void getStationsWith( StationList & out, Functor & func );
}


#include "getStations.tcc"

#endif /*GETSTATIONS_H_*/
