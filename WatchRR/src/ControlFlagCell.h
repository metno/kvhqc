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
#ifndef __WatchRR__ControlFlagCell_h__
#define __WatchRR__ControlFlagCell_h__

#include "RRTableItem.h"
#include <kvData.h>

namespace WatchRR
{
  class RRTable;

  namespace cell
  {
    class ControlFlagCell
      : public RRTableItem
    {
    public:
      ControlFlagCell( Q3Table * t, const kvalobs::kvData & data );
      virtual ~ControlFlagCell( );
      
      virtual QString explain() const;

      QString fullText() const { return fullText_; }
      QString abbrevText() const { return abbrevText_; }

      /**
       * recalculate and display text for cell, tooltip and statusbar,
       * if eg. the underlying data object has changed.
       */
      void recalculateTexts();

      /**
       * resignal for cell to display correct text
       */
      void setText();

      using Q3TableItem::setText;

      virtual int alignment() const { return Qt::AlignLeft | Qt::AlignVCenter; }
      
    private:
      const kvalobs::kvData & data;

      QString fullText_;
      QString abbrevText_;
    };
  }
}

#endif // __WatchRR__ControlFlagCell_h__
