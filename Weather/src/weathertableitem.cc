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
#include "weathertableitem.h"
#include "weathertable.h"
#include <kvalobs/kvDataOperations.h>
#include <cmath>
#include <cassert>
using namespace kvalobs;
using namespace std;

namespace Weather
{
  class WeatherTable;

  //WeatherTableItem::WeatherTableItem( QTable * table, kvData & data )
  WeatherTableItem::WeatherTableItem( QTable* table, QTableItem::EditType edType, QString type, QString flag)
    : QTableItem( table, edType, flag )
    , type(type)
  {
  }
  
  WeatherTableItem::~WeatherTableItem()
  {
  }
  
  QString WeatherTableItem::explain() const
  {
    QString ret = text();
    if ( ret.isEmpty() )
      ret = "Ingen data (original fra stasjon: manglende)";
    else
      ret = "TypeId = " + type;
    return ret;
  }

  void WeatherTableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected )
  {
    QColorGroup g( cg );
    if ( isModelVal )
      g.setColor( QColorGroup::Text, red );
    else
      g.setColor( QColorGroup::Text, black );
    
    QTableItem::paint( p, g, cr, selected );
  }
}
