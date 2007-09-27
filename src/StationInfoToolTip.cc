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
#include "StationInfoToolTip.h"
#include "StationInformation.h"
#include "TypeInformation.h"
#include <KvApp.h>

typedef StationInformation<kvservice::KvApp> StationInfo;
typedef TypeInformation<kvservice::KvApp>    TypeInfo;

using namespace kvservice;
using namespace std;

StationInfoToolTip::StationInfoToolTip( QTable *table, QToolTipGroup *group, 
					int stationidCol, int typeidCol )
  : QToolTip( table->viewport(), group )
  , table( table )
  , stationidCol( stationidCol )
  , typeidCol( typeidCol )
{
}

StationInfoToolTip::~StationInfoToolTip( )
{
}

void StationInfoToolTip::maybeTip ( const QPoint &p )
{
  QPoint cp = table->viewportToContents( p );
  int row = table->rowAt( cp.y() );
  int col = table->columnAt( cp.x() );


  QString cellText = table->text( row, stationidCol );
  if ( cellText.isNull() )
    return;
     
  bool ok = true;
  QString tipString = 
    StationInfo::getInstance(KvApp::kvApp)->getInfo( cellText.toInt( &ok ) );
  if ( !ok ) {// Cold not convert cell contents to int.
    return;
  }

  cellText = table->text( row, typeidCol );
  if ( cellText.isNull() )
    return;
  ok = true;
  tipString += " - " + TypeInfo::getInstance(KvApp::kvApp)->getInfo( cellText.toInt( &ok ) );
  if ( !ok ) { // Cold not convert cell contents to int.
    return;
  }

  QRect cr = table->cellGeometry( row, col );
  cr.moveTopLeft( table->contentsToViewport( cr.topLeft() ) );
  tip( cr, tipString );
}
