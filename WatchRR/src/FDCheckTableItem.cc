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
#include "FDCheckTableItem.h"
#include "RR_24DataTableItem.h"
#include "RRTable.h"
#include <kvalobs/kvDataOperations.h>
#include <cmath>
#include <cassert>

using namespace kvalobs;
using namespace std;

namespace WatchRR
{

FDCheckTableItem::FDCheckTableItem( Q3Table * table, kvData & data )
  : RRCheckTableItem2( table, data )
{
  setReplaceable( false );
  setChecked( collected() );
}

FDCheckTableItem::~FDCheckTableItem()
{
}

bool FDCheckTableItem::collected() const
{
  kvalobs::kvControlInfo cinfo = data.front()->controlinfo();
  int fd = cinfo.flag( kvalobs::flag::fd );
  return fd == 2 or fd > 5;
}

QString FDCheckTableItem::explain() const
{
  if ( isChecked() )
    return "Nedbør er en del av en oppsamling.";
  else
    return "Nedbør er ikke en del av en oppsamling.";
}

void FDCheckTableItem::getUpdatedList( DataSet & data )
{
  if ( isChecked() != collected() ) {
    kvData d = * getKvData().front();
    cerr << "Modified: Obstime: " << d.obstime() << ", flag: " << d.controlinfo().flag( kvalobs::flag::fd ) << endl;
    DataSet::iterator f = data.find( d );
    if ( f != data.end() ) {
      d = * f;
      data.erase( f );
    }

    kvalobs::kvControlInfo ci = d.controlinfo();
    int fd = 1;
    if ( isChecked() ) {
      if ( not kvalobs::missing( d ) and std::abs( d.original() - d.corrected() ) > 0.0625 )
        fd = 6;
      else
        fd = 2;
      ci.set( kvalobs::flag::fd, fd );      
    }
    
    ci.set( kvalobs::flag::fd, fd );
    ci.set( kvalobs::flag::fhqc, 0 );
    d.controlinfo( ci );

    miutil::miString cf = d.cfailed();
    if ( not cf.empty() )
      cf += ",";
    //    cf += "Manuelt skjønn";
    cf += "Manual judgment";
    d.cfailed(cf);
    //    kvalobs::hqc::hqc_auto_correct( d, d.corrected() ); // set correct fhqc flag
  	
    data.insert( d );
  }
}

}
