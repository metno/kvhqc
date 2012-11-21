/* -*- c++ -*-

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
#ifndef __ErrorListFirstCol_h__
#define __ErrorListFirstCol_h__

#include <q3table.h>
#include "errorlist.h"


/**
 * \brief The first column of the error list.
 *
 * This cell contains a mark indicating if it represents the same
 * station as the one currently selected in the error list.
 *
 * Also, it contains a workaround for possible inconsistencies between
 * indexes in the error list and the errorlist's memStore3. The row's
 * index in memStore3 can be fetched with the memStoreIndex() method.
 */
class ErrorListFirstCol
  : public Q3TableItem
{
  const int memStorePlace;
public:
  ErrorListFirstCol( ErrorList *el, int memStorePlace )
    : Q3TableItem( el, Q3TableItem::Never, "" )
    , memStorePlace( memStorePlace )
  {
    setReplaceable(false);
  }

  int memStoreIndex() const
  {
    return memStorePlace; 
  }

  void setSameStation( bool same )
  {
    if ( same )
      setText( ">" );
    else
      setText( " " );
  }
};

#endif // __ErrorListFirstCol_h__
