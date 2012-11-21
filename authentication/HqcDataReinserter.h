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
#ifndef __HqcDataReinserter_h__
#define __HqcDataReinserter_h__

#include <decodeutility/DataReinserter.h>
//#include <kvQtApp.h>
#include <kvcpp/KvApp.h>

class HqcDataReinserter
  : public kvalobs::DataReinserter<kvservice::KvApp>
{
public:
  HqcDataReinserter( kvservice::KvApp *app, int operatorID );
  virtual ~HqcDataReinserter( );

  virtual const CKvalObs::CDataSource::Result_var
  insert( kvalobs::kvData &d ) const;

  virtual const CKvalObs::CDataSource::Result_var
  insert( std::list<kvalobs::kvData> &dl ) const;
};

#endif // __HqcDataReinserter_h__
