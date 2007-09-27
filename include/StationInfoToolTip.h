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
#ifndef __StationInfoToolTip_h__
#define __StationInfoToolTip_h__
#include <qtooltip.h>
#include <qtable.h>
//#include <kvservice/qt/kvQtApp.h>
#include <KvApp.h>



class StationInfoToolTip
  : public QToolTip
{
public:
  StationInfoToolTip( QTable *table, QToolTipGroup *group = 0, 
		      int stationidCol = 1, int typeidCol = 7 );

  virtual ~StationInfoToolTip( );

protected:
  void maybeTip( const QPoint &p );

private:
  QTable *table;

  // A possible error in this class will be if these values does not
  // point to the correct columns:
  int stationidCol;
  int typeidCol;
};

#endif // __StationInfoToolTip_h__
