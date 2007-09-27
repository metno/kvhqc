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
#ifndef __StationInformation_h__
#define __StationInformation_h__

#include <kvalobs/kvData.h>
#include <kvalobs/kvStation.h>
#include <ios>
#include <map>
#include <qstring.h>

template<class App>
class StationInformation
{
public:
  
  static const StationInformation *getInstance( App *app = NULL );

  virtual ~StationInformation( );

  virtual QString getInfo( int stationID ) const;

  inline  QString getInfo( const kvalobs::kvData &data ) const
  { return getInfo( data.stationID() ); }

  virtual QString getAll() const;

  const kvalobs::kvStation *operator[]( int stationID ) const;

  inline const kvalobs::kvStation *operator[]( const kvalobs::kvData &data ) const
  { return this[ data.stationID() ]; }

protected:
  StationInformation( App *app );
  static StationInformation *stInfo;
  std::map<int, kvalobs::kvStation> stations;
};

#include "bits/StationInformation.tcc"

#endif // __StationInformation_h__
