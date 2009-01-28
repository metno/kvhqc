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
#ifndef __WatchRR__snow__SDDataTableItem_h__
#define __WatchRR__snow__SDDataTableItem_h__

#include "RRTableItem.h"
#include <dataconsistencyverifier.h>
#include <qstringlist.h>

namespace WatchRR
{
  namespace snow
  {
    /**
     * \brief A Table item for displaying a non-editable SD value.
     */

    /**
     * \brief A Table item for displaying an editable SD value.
     */
    class SDDataTableItem
      : public RRComboTableItem
    {
    public:
      SDDataTableItem( Q3Table *table, kvalobs::kvData & sd );
      SDDataTableItem( Q3Table *table, float value );
      virtual ~SDDataTableItem( );

      virtual QString explain( ) const;
      
    protected:
      virtual QString getText( float f ) const;
      
      using RRTableItem::getText;

    private:
      static QStringList selections;
      static QStringList selectionsWUnknown;
    };
  }
}

#endif // __WatchRR__snow__SDDataTableItem_h__
