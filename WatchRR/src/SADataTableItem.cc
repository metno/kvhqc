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
#include "SADataTableItem.h"
#include <qlineedit.h>

#include <cassert>

using namespace kvalobs;

namespace WatchRR
{
  namespace snow
  {
    const QRegExp SADataTableItem::re( "([0-9]+)|(\\-[13]?)" );
    const QRegExpValidator SADataTableItem::validator( SADataTableItem::re, NULL );


    SADataTableItem::SADataTableItem( Q3Table * table, kvalobs::kvData & data )
      : RRTableItem( table, data, & validator )
    {
        setText( getText() );
    }

    SADataTableItem::SADataTableItem( Q3Table * table, float value )
      : RRTableItem( table )
    {
      setText( getText( value ) );
    }


    QString SADataTableItem::explain() const
    {
      QString ret;
      QString txt = text();
      float f = txt.toFloat();
      if ( txt == missingDataIdentifier )
        ret =  "Manglende observasjon";      
      else if ( f == noSnow )
        ret = "Ingen snødybde å melde / ingen snø";
      else if ( f == impossibleToMeasure )
        ret = "Umulig å måle snødybde";
      else if ( f == 0 )
      	ret = "Snødybde < 0.5 cm";
      else if ( f < 0 )
  	    ret = "Ukjent verdi";
      else
        ret = txt + " cm snødybde";

      if ( modelValue_ )
        ret += " (modellverdi)";
      return ret + ".";
    }
    
    SADataTableItem::~SADataTableItem( )
    {
    }
    
    QString SADataTableItem::getText( float f ) const
    {
      if ( f == CellValueProvider::missing )
        return missingDataIdentifier;
      return QString::number( (int) f );
    }
  }
}
