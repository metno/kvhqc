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

namespace Weather
{

WeatherTableItem::WeatherTableItem( Q3Table* table, Q3TableItem::EditType edType, QString type, QString flag)
    : Q3TableItem( table, edType, flag )
    , type(type)
{
}

WeatherTableItem::~WeatherTableItem()
{
}

QString WeatherTableItem::explain() const
{
    QString ret = text();
    if ( text().isEmpty() )
        return QObject::tr("Ingen data (original fra stasjon: manglende)");
    else
        return QObject::tr("TypeId = %1").arg(type);
}

void WeatherTableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected )
{
    QColorGroup g( cg );
    if( isModelVal )
        g.setColor( QColorGroup::Text, Qt::red );
    else if ( isCorrectedByQC2 )
        g.setColor( QColorGroup::Text, Qt::darkMagenta );
    else
        g.setColor( QColorGroup::Text, Qt::black );
    
    Q3TableItem::paint( p, g, cr, selected );
}

}
