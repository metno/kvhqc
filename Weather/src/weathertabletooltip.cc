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

#include "weathertabletooltip.h"

#include "selfexplainable.h"
#include "weathertable.h"

#include <QtCore/QString>

namespace Weather
{
  WeatherTableToolTip::WeatherTableToolTip( const WeatherTable *table)
    : table( table )
  {
  }

  void WeatherTableToolTip::maybeTip( const QPoint &p )
  {
    QPoint cp = table->viewportToContents( p );
    int row = table->rowAt( cp.y() );
    int col = table->columnAt( cp.x() );

    Q3TableItem *ttItem = table->item(row, col);
    if ( ttItem == NULL ) 
      return;

    QString tipString = "";

    SelfExplainable *se = 
      dynamic_cast<SelfExplainable *>( ttItem );
    if ( se == NULL ) {
      tipString = ttItem->text();
    }
    else {
      tipString = se->explain();
    }
    QRect cr = table->cellGeometry( row, col );    
    cr.moveTopLeft( table->contentsToViewport( cr.topLeft() ) );
  }
}
