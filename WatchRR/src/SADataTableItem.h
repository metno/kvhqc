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
#ifndef __WatchRR__snow__SADataTableItem_h__
#define __WatchRR__snow__SADataTableItem_h__

#include "RRTableItem.h"
#include "dataconsistencyverifier.h"

namespace WatchRR
{
  namespace snow
  {
    class SADataTableItem
      : public RRTableItem
    {
    public:
      SADataTableItem( Q3Table * table, kvalobs::kvData & data );

      SADataTableItem( Q3Table * table, float value );

      virtual ~SADataTableItem( );

      virtual QString explain() const;

    protected:
      virtual QString getText( float f ) const;
      using RRTableItem::getText;
    
      static const QRegExpValidator validator;
      static const QRegExp re;

      /**
       * \brief Identifies missing data.
       */
      //static const QString missingDataIdentifier;

      static const float noSnow = -1;
      static const float impossibleToMeasure = -3;

      /**
       * \brief Identifies a _complete_ lack of snow.
       */
      static const QString noSnowIdentifier;

      /**
       * \brief Measuring of snow depth could not be done.
       */
      static const QString impossibleToMeasureIdentifier;
    };
  }
}

#endif // __WatchRR__snow__SADataTableItem_h__
