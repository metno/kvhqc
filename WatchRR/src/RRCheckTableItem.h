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
#ifndef RRCHECKTABLEITEM_H_
#define RRCHECKTABLEITEM_H_

#include <qtable.h>
#include "selfexplainable.h"

namespace kvalobs {
  class kvData;
}

namespace WatchRR
{
  
class RR_24DataTableItem;

class RRCheckTableItem
  : public QCheckTableItem
  , public SelfExplainable
{
public:
	RRCheckTableItem( QTable * table, RR_24DataTableItem * titem );
	virtual ~RRCheckTableItem();
  
  virtual QString explain() const;

  virtual void updateChecked();
  
  virtual void setContentFromEditor ( QWidget * w );
  
private:
  RR_24DataTableItem * dataholder_;
};

}

#endif /*RRCHECKTABLEITEM_H_*/
