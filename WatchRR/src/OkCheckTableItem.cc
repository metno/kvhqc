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
#include "OkCheckTableItem.h"

OkCheckTableItem::OkCheckTableItem( Q3Table * table, KvDataProvider::Data & data )
  : RRCheckTableItem2( table, data )
{
}

OkCheckTableItem::~OkCheckTableItem()
{
}

QString OkCheckTableItem::explain() const
{
  if ( isChecked() )
    return "Godkjent";
  else
    return "";
}

namespace {
  inline bool shouldChange_( const kvalobs::kvData & d )
  {
    int fhqc = d.controlinfo().flag( kvalobs::flag::fhqc );
    return fhqc == 0 or fhqc == 2;
  }
}

void OkCheckTableItem::getUpdatedList( DataSet & out )
{
  if ( isChecked() ) {
    // Mark all otherwise unmodified data as OK (fhqc = 1)
    for ( KvDataProvider::Iterator it = data.begin(); it != data.end(); ++ it ) {
      if ( not kvalobs::missing( ** it ) 
           and out.find( ** it ) == out.end()
           and shouldChange_( ** it ) ) {
        kvalobs::kvData d( ** it );
        kvalobs::hqc::hqc_accept( d );
        out.insert( d );
      }
    }
  }
}
