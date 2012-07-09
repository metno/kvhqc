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
#include "HqcDataReinserter.h"
#include <kvalobs/kvDataOperations.h>
#include <algorithm>

using namespace kvservice;
using namespace kvalobs;

namespace internal_
{
  void addCFailed( kvalobs::kvData &d ) 
  {
    miutil::miString cf = d.cfailed();
    const kvalobs::kvControlInfo cinfo = d.controlinfo();
    if ( cinfo.flag( flag::fhqc ) >= 2 ) {
      if ( cf.empty() )
	cf = "hqc";
      else if ( !cf.contains("watchweather") && 
		!cf.contains("watchRR") )
	cf += ",hqc";
    }
    else if ( cinfo.flag( flag::fhqc ) == 1 && 
	      !cf.empty() && 
	      !cf.contains("watchweather") &&
	      !cf.contains("watchRR") )
      cf += ",hqc";
    else if ( cinfo.flag( flag::fhqc ) == 0 &&
	      !cf.empty() && 
	      !cf.contains("watchweather") &&
	      !cf.contains("watchRR") &&
	      d.corrected() == d.original() &&
	      d.corrected() == -32767 ) {
      cf += ",hqc";
    }
  
    d.cfailed( cf );
    kvUseInfo ui = d.useinfo();
    ui.addToErrorCount();
    d.useinfo( ui );
  }

  void updateUseInfo( kvalobs::kvData & d )
  {
    const kvalobs::kvControlInfo ci = d.controlinfo();
    kvalobs::kvUseInfo ui = d.useinfo();
    miutil::miString cf = d.cfailed();

    if (( cf.contains("hqc") || cf.contains("watchweather") || cf.contains("watchRR")) && ci.flag( flag::fhqc ) == 0 ) {
      kvalobs::kvControlInfo tci = d.controlinfo();
      tci.set(15,1);
      ui.setUseFlags( tci );
    }
    else {
      ui.setUseFlags( ci );
    }
    d.useinfo( ui );
  }
}

HqcDataReinserter::HqcDataReinserter( KvApp *app, int operatorID )
  : DataReinserter<KvApp>( app, operatorID )
{
}

HqcDataReinserter::~HqcDataReinserter( )
{
}

const CKvalObs::CDataSource::Result_var
HqcDataReinserter::insert( kvalobs::kvData &d ) const
{
  ::internal_::addCFailed( d );
  ::internal_::updateUseInfo( d );
  return DataReinserter<KvApp>::insert( d );
}


const CKvalObs::CDataSource::Result_var
HqcDataReinserter::insert( std::list<kvalobs::kvData> &dl ) const
{
  std::for_each( dl.begin(), dl.end(), ::internal_::addCFailed );
  std::for_each( dl.begin(), dl.end(), ::internal_::updateUseInfo );
  return DataReinserter<KvApp>::insert( dl );
}
