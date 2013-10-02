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

#ifndef FINDMISSINGSTATIONS_H_
#define FINDMISSINGSTATIONS_H_


#include "MissingList.hh"
#include "TaskSpecification.hh"

  
/**
 * Find those stations matching a TaskSpecification that have not arrived yet.
 * 
 * \param[out] ml The list to be filled with missing stations.
 * \param ts The specification of what kind of observations to check.
 * \param stop If set to true by another thread, execution of this function
 *             will stop, even if unfinished.
 *
 * \ingroup group_data_analysis
 */
void findMissingStations( MissingList & ml, const TaskSpecification & ts, const bool & stop );

#endif /*FINDMISSINGSTATIONS_H_*/
