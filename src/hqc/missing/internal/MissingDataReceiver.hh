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

#ifndef MISSINGDATARECEIVER_H_
#define MISSINGDATARECEIVER_H_

#include <kvcpp/KvGetDataReceiver.h>
#include "MissingList.hh"
#include "TaskSpecification.hh"


namespace FindMissing
{

/**
 * Remove data from a list of possibly missing observations. As objects of
 * this class receives data, they will remove the coresponding data from its
 * %internal possibly-missing list. After all data have been processed, the
 * given list of missing stations, will only contain those who have not given
 * any observations.
 *
 * A list of missing observations, which have later been filled with
 * observations with valid periods longer than their usual 24 hours will also
 * be filled in.
 *
 * \ingroup group_data_analysis
 */
class MissingDataReceiver
      : public kvservice::KvGetDataReceiver
{
    ExStationList * missing;
    ExStationList * collected;
    const TaskSpecification & ts;
    const bool * stop;
    bool processKvData_;
    bool processTextData_;

    // Will also populate collection list
    class remove_if_not_missing
    {
        ExStationList * missing;
        ExStationList * collected;
        const TaskSpecification & ts;
        template<class Data>
        bool isRelevant( const Data & data ) const;
        bool is_missing( const kvalobs::kvData & data ) const;
      public:
        remove_if_not_missing( ExStationList * missing, ExStationList * collected,
                               const TaskSpecification & ts );
        void operator()( const kvalobs::kvData & data );
        void operator()( const kvalobs::kvTextData & textData );
    };


  public:
    /**
     * \param[in,out] missing A list of possible stations. Each station will be
     *                        removed if data for the period has arrived.
     * \param[out] collected To be populated with information about
     *                       observation collection (i a single observation is
     *                       valid for several periods).
     * \param ts Specification of data to search for
     * \param stop May later be set to true. This will then stop receiving data
     */
    MissingDataReceiver( ExStationList * missing, ExStationList * collected,
                         const TaskSpecification & ts, const bool * stop = NULL );

    virtual ~MissingDataReceiver();

    /**
     * Removed stations from the given list from %internal collection of
     * possibly missing stations.
     *
     * Implementation from kvservice::KvGetDataReceiver::next
     */
    virtual bool next( kvservice::KvObsDataList & datalist );
};

}

#endif /*MISSINGDATARECEIVER_H_*/
