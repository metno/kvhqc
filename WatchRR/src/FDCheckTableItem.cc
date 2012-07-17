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
#include "hqc_utilities.hh"
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
  const kvalobs::kvControlInfo cinfo = data.front()->controlinfo();
  const int fd = cinfo.flag( kvalobs::flag::fd );
  return fd == 2 or fd == 4 or fd >= 6;
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
    const bool dbCollected = collected();
    const bool hqcCollected = isChecked();
    if ( hqcCollected == dbCollected )
        return;

    kvData d = * getKvData().front();
    const int dbFlagFd = d.controlinfo().flag( kvalobs::flag::fd );
    DataSet::iterator f = data.find( d );
    if ( f != data.end() ) {
        d = * f;
        data.erase( f );
    }
    cerr << "Modified: Obstime: " << d.obstime() << ", flag: " << dbFlagFd << endl;

    // 'isEndpoint' should be true iff the observation contains the collected
    // value and thus is the last point of an accumulation
    const bool isEndpoint = (not kvalobs::original_missing(d));

    // 'hasNewCorrected' should be true iff the operator has manually set a new
    // corrected value in addition to changing the "collected" checkmark
    const bool hasNewCorrected = (std::abs(d.original() - d.corrected()) > 0.0625);

    int newFd, newFhqc;
    if ( hqcCollected ) {
        if( hasNewCorrected ) {
            newFd = isEndpoint ? 0xA : 9;
            newFhqc = 6;
        } else {
            newFd = isEndpoint ? 4 : 2;
            newFhqc = 4;
        }
    } else {
        newFd = 1;
        newFhqc = ( hasNewCorrected ) ? 7 : 1;
    }

    kvalobs::kvControlInfo ci = d.controlinfo();
    ci.set( kvalobs::flag::fd, newFd );
    ci.set( kvalobs::flag::fhqc, newFhqc );
    d.controlinfo( ci );

    data.insert( d );
}

}
