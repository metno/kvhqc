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
#include "dataconsistencyverifier.h"
#include "enums.h"
#include <cmath>

namespace WatchRR
{
  void DataConsistencyVerifier::get_inconsistent( DataList & out ) const
  {
    KvDataProvider::Data    kd = getKvData();
    CellValueProvider::Data cv = getCellValue();
    KvDataProvider::Iterator    kdi = kd.begin();
    CellValueProvider::Iterator cvi = cv.begin();

    for ( ; kdi != kd.end(); ++ kdi, ++ cvi )
    {
      const kvalobs::kvData & original_data = ** kdi;
      const float represented_value = * cvi;

      if ( not equal( original_data, represented_value ) )
      {
        kvalobs::kvData d( original_data );

        if ( represented_value == CellValueProvider::missing )
          kvalobs::hqc::hqc_reject( d );
	//	else
	//	  kvalobs::hqc::hqc_auto_correct( d, represented_value );
	
	else if ( fabs(d.original() - represented_value) > 0.0001 ) 
	  kvalobs::hqc::hqc_auto_correct( d, represented_value );
	else {
	  kvalobs::kvControlInfo ctr = d.controlinfo();
	  ctr.set(6,0);
	  d.corrected(represented_value);
	  d.controlinfo(ctr);
	  kvalobs::hqc::hqc_accept(d);
	} 
	out.push_back( d );
      }
    }
  }

  namespace
  {
    template<typename Set>
    struct insert_replace
    {
      Set & container;
      insert_replace( Set & c ) : container( c ) {}
      void operator () ( typename Set::value_type & val )
      {
        typedef typename Set::iterator iterator;
        std::pair<iterator, bool> res = container.insert( val );
        if ( ! res.second )
        {
          container.erase( res.first );
          container.insert( val );
        }
      }
    };
  }

  void DataConsistencyVerifier::getUpdatedList( DataSet & data )
  {

    DataList modified;
    get_inconsistent( modified );

    for_each( modified.begin(), modified.end(), insert_replace<DataSet>( data ) );
  }
  bool DataConsistencyVerifier::equal( const kvalobs::kvData & d, float v ) const
  {
    if ( v == CellValueProvider::missing )
      return not kvalobs::valid( d );
    return kvalobs::valid( d ) and std::abs( d.corrected() - v ) < 0.0625;
  }
}
