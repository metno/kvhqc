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
#include "RR_24DataTableItem.h"
#include "RRTable.h"
#include "enums.h"
#include <qlineedit.h>
#include <qmessagebox.h>
#include <kvalobs/kvDataOperations.h>

using namespace std;
using namespace kvalobs;

namespace WatchRR
{
  const QString RR_24DataTableItem::standardType = "observasjon";

  const QString RR_24DataTableItem::missingDataIdentifier = "-";

  const QRegExp RR_24DataTableItem::re( "([0-9]+([.,][0-9])?)|(\\-1?)" );

  const QRegExpValidator
  RR_24DataTableItem::validator( RR_24DataTableItem::re, NULL );


  RR_24DataTableItem::
  RR_24DataTableItem( Q3Table *table, kvData & data)
    : RRTableItem( table, data, & validator )
    , type( standardType )
  {
    setText( getText() );
  }

  RR_24DataTableItem::
  RR_24DataTableItem( Q3Table * table, float value, QString type )
    : RRTableItem( table )
    , type( type )
  {
    setText( getText( value ) );
  }


  QString RR_24DataTableItem::explain() const
  {
    QString ret;
    QString txt = text();
    float val;

    if ( txt == missingDataIdentifier )
      val = CellValueProvider::missing;
    else
      val = txt.toFloat();
    if ( val == CellValueProvider::missing )
      ret = "Manglende " + type;
    else if ( val < 0 )
      ret = "Ingen nedbør siste døgn";
    else
      ret = txt + " mm nedbør siste 24 timer";
    if ( modelValue_ )
      ret += " (modellverdi)";
    return ret + '.';
  }

  void RR_24DataTableItem::getKvData( KvDataProvider::Data & dataList ) const
  {
    dataList.insert( dataList.end(), data.begin(), data.end() );
  }

}
