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
#include "RRCheckTableItem.h"
#include "RR_24DataTableItem.h"
#include "RRTable.h"
#include <kvalobs/kvDataOperations.h>

#include <cassert>

using namespace kvalobs;

namespace WatchRR
{
  
  RRCheckTableItem::RRCheckTableItem( Q3Table * table, RR_24DataTableItem * titem )
    : Q3CheckTableItem( table, "" )
    , dataholder_( titem )
  {
    setReplaceable( false );
    updateChecked();
  }
  
  RRCheckTableItem::~RRCheckTableItem()
  {
  }
  
  QString RRCheckTableItem::explain() const
  {
    if ( isChecked() )
      return "Nedbør er en del av en oppsamling.";
    else
      return "Nedbør er ikke en del av en oppsamling.";
  }
  
  void RRCheckTableItem::updateChecked()
  {
    setChecked( dataholder_->collected() );
  }
  
  void RRCheckTableItem::setContentFromEditor( QWidget * w )
  {
    Q3CheckTableItem::setContentFromEditor( w );
    dataholder_->setCollected( isChecked() );
    
    RRTable * rrt = dynamic_cast<RRTable *>( table() );
    if ( rrt ) {
      rrt->markModified( dataholder_ );
    }
  } 
}
