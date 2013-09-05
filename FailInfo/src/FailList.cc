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

#include "cFailedParam.h"
#include "explainQC.h"
#include "mi_foreach.hh"

#include <Qt3Support/Q3ListView>

#include <map>
#include <set>

#define MILOGGER_CATEGORY "kvhqc.FailList"
#include "HqcLogging.hh"

using namespace std;
using namespace kvalobs;

namespace FailInfo
{
  FailList::FailList( QWidget *parent, const char * name, Qt::WFlags f )
      : QWidget(parent, name, f), Ui_cFailedWidget()
  {
    setupUi(this);
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
    
    METLIBS_LOG_SCOPE();

    cfailedList->clear();
    
    QC::cFailList fail = QC::getFailList( data.cfailed() );
    if( fail.empty() )
        return;

    Fails fails;
    mi_foreach(const QC::cFailedParam& cf, fail) {
        fails
            [ QString::fromStdString(cf.getPart( QC::cFailedParam::QcClass ))   ]
            [ QString::fromStdString(cf.getPart( QC::cFailedParam::Group )) ]
            .insert( cf );
    }
    mi_foreach(Fails::value_type f, fails) {
        Q3ListViewItem *topItem =
            new Q3ListViewItem( cfailedList, f.first, "" );
        topItem->setOpen(true);
        METLIBS_LOG_DEBUG(LOGVAL(f.first));
        QC::FailGroupList &failGroupList = QC::failExpl[ f.first.toStdString() ];
        
        mi_foreach(const SubElem::value_type& sub, f.second) {
            QC::FailGroup &failGroup = failGroupList[sub.first.ascii()];
            METLIBS_LOG_DEBUG(LOGVAL(sub.first));
            Q3ListViewItem *subItem =
                new Q3ListViewItem( topItem, sub.first,
                                    QString::fromStdString(failGroup.explanation) );
            subItem->setOpen(true);
            
            mi_foreach( const QC::cFailedParam& subsub, sub.second) {
                METLIBS_LOG_DEBUG("subsub detail='" << subsub.getPart( QC::cFailedParam::Detail ));
                new Q3ListViewItem( subItem,
                                    QString::fromStdString(subsub.getPart( QC::cFailedParam::Detail )),
                                    QString::fromStdString(failGroup.getDetailExplanation(subsub, data)) );
            }
        }
    }
  }
}
