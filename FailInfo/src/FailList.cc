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
#include "FailList.h"
#include "explainQC.h"
#include "cFailedParam.h"
#include <puTools/miString>
#include <miutil/commastring.h>
#include <map>
#include <set>
#include <qlistview.h>

#include <iostream>

using namespace std;
using namespace kvalobs;
using namespace miutil;

namespace FailInfo
{
  FailList::FailList( QWidget *parent, const char * name, WFlags f )
    : cFailedWidget(parent, name, f)
  {
  }
  
  FailList::~FailList( )
  {
  }

  QSize FailList::sizeHint () const
  {
    return QSize( 384, 128 );
  }

  typedef map< QString, set<QC::cFailedParam> >  SubElem;
  typedef map< QString, SubElem > Fails;

  void FailList::newData( const kvalobs::kvData & data ) 
  {
    if ( data == this->data )
      return;

    this->data = data;

    cfailedList->clear();

    QC::cFailList fail = QC::getFailList( data.cfailed() );
    if ( fail.empty() )
      return;

    Fails fails;

    for ( QC::cFailList::iterator it = fail.begin();
	  it != fail.end();  it++ ) 
      fails
	[ it->getPart( QC::cFailedParam::QcClass )   ]
	[ it->getPart( QC::cFailedParam::Group ) ]
	.insert( *it );

    for ( Fails::const_iterator top = fails.begin();
	  top != fails.end();  top++ ) {
      QListViewItem *topItem = 
	new QListViewItem( cfailedList, top->first, "" );
      topItem->setOpen(true);
      QC::FailGroupList &failGroupList = QC::failExpl[ top->first ];

      for ( SubElem::const_iterator sub = top->second.begin();
	    sub != top->second.end();  sub++ ) {
	QC::FailGroup &failGroup = 
	  failGroupList[sub->first.ascii()];
	QListViewItem *subItem =
	  new QListViewItem( topItem, sub->first, 
			     failGroup.explanation );
	subItem->setOpen(true);

	for ( set<QC::cFailedParam>::const_iterator subsub = sub->second.begin();
	      subsub != sub->second.end();  subsub++ ) {
	  new QListViewItem( subItem, 
			     subsub->getPart( QC::cFailedParam::Detail ), 
			     failGroup.getDetailExplanation( *subsub, data) );
	}
      }
    }
    // increase distance between failnumber and explanation:
    //    cfailedList->setColumnWidth( 0, cfailedList->columnWidth(0) + 10 );
  }
}
