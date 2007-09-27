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
#include "SDDataTableItem.h"
#include <iostream>

using std::endl;
using std::cout;

namespace WatchRR
{
  namespace snow
  {
    QString unknownValueIdentifier = "?";
/*
    SDDataTableItem_Simple::SDDataTableItem_Simple( QTable * table, float value )
      : RRTableItem( table, true )
      , explanation( internal::explain( value ) )
    {
      //QString::number( value ) );
      setText( getText( value ) );
    }

    QString SDDataTableItem_Simple::explain( ) const
    {
      return explanation;
    }
*/

    QStringList SDDataTableItem::selections;
    QStringList SDDataTableItem::selectionsWUnknown;

    SDDataTableItem::SDDataTableItem( QTable *table, kvalobs::kvData & sd )
      : RRComboTableItem( table, sd, selections )
    {
      setReplaceable( false );

      if ( selections.isEmpty() ) {
      	selections.push_back( "-" );
      	selections.push_back( "-1" );
      	for ( int i = 1; i < 5; i++ )
      	  selections.push_back( QString::number(i) );
        
        selectionsWUnknown = QStringList( selections );
        selectionsWUnknown.push_back( unknownValueIdentifier );
      }

      QString txt = getText();
      if ( txt == unknownValueIdentifier )
        setValues( & selectionsWUnknown );
      
      setText( txt );

      /*
      if ( selections.find( setTo ) != selections.end() )
      	setStringList( selections );
      else {
      	QStringList newList( selections );
      	newList.append(unknownValueIdentifier  );
      	setStringList( newList );
      	setTo = unknownValueIdentifier;
      }
      setCurrentItem( setTo );
      */
    }
    
    SDDataTableItem::SDDataTableItem( QTable *table, float value )
      : RRComboTableItem( table )
    {
      setText( getText( value ) );
    }
    
    SDDataTableItem::~SDDataTableItem( )
    {
    }

    QString SDDataTableItem::explain() const
    {
      
      float f;
      if ( not data.empty() )
        f = getCellValue().front();
      else {
        QString txt = text();
        if ( txt == missingDataIdentifier )
          f = CellValueProvider::missing;
        else
          f = txt.toFloat();
      }
        
      switch ( (int) f ) {
        //case 0:
      case -1:
        return "Snøbart.";
      case 1:
        return "Flekkvis snø.";
      case 2:
        return "Omkring 50% snødekke.";
      case 3:
        return "Hovedskelig dekket av snø - enkelte bare flekker.";
      case 4:
        return "Helt snødekket.";
      default:
        if ( f == CellValueProvider::missing )
          return "Manglende observasjon";
        return "Ukjent verdi (" + QString::number( f ) + ").";
      }
    }
    
    QString SDDataTableItem::getText( float f ) const
    {
      if ( f == CellValueProvider::missing )
        return missingDataIdentifier;
      if ( f < -1 or f > 4 or f == 0 )
        return unknownValueIdentifier;
      return QString::number( f, 'f', 0 );
    }
  }
}
