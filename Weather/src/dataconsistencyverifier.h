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
#ifndef __Weather__DataConsistencyVerifier_h__
#define __Weather__DataConsistencyVerifier_h__

#include "cellvalueprovider.h"
#include "kvdataprovider.h"
#include <kvalobs/kvDataOperations.h>

namespace Weather
{
  class DataConsistencyVerifier
        : public CellValueProvider
        , public KvDataProvider
  {
  public:
    using CellValueProvider::getCellValue;
    using KvDataProvider::getKvData;

    typedef std::set<kvalobs::kvData, kvalobs::compare::lt_kvData> DataSet;
    typedef std::list<kvalobs::kvData> DataList;
    
    /**
     * Get the list of table objects whose shown value is inconsistent with 
     * the base data (ie. get the data that may need to be saved.
     * @param out The return list. Will contain kvData objects, containing
     *             updated values.
     */
    void get_inconsistent( DataList & out ) const;


    virtual void getUpdatedList( DataSet & data );
    
    protected:
      /**
       * Are the corrected value of d considered to be equal to v?
       */
      virtual bool equal( const kvalobs::kvData & d, float v ) const;
  };
}

#endif // __Weather__DataConsistencyVerifier_h__
