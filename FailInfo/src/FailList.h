/* -*- c++ -*-
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2007-2012 met.no

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
#ifndef __FailInfo__FailList_h__
#define __FailInfo__FailList_h__

#include "ui_cfailedwidget.h"
#include <kvalobs/kvData.h>
#include <qstring.h>

namespace FailInfo
{
  /**
   * \brief The implementation of the GUI part of the fail list.
   */
  class FailList
    : public QWidget, public Ui_cFailedWidget
  {
    Q_OBJECT;
  public:
    /**
     */
    FailList( QWidget * parent = 0, const char * name = 0, Qt::WFlags f = 0 );
    //    FailList( QWidget * parent = 0 );
    virtual ~FailList( );
    virtual QSize sizeHint () const;

  public slots:
    /**
     * \brief Triggers an update of the FailList widget, unless \c
     * data's errors was already displayed there from before.
     *
     * \param data The \c kvData object from which to generate error
     * information.
     */
    virtual void newData( const kvalobs::kvData & data );

  protected:
    /**
     * \brief The kvData object currently being displayed.
     */
    kvalobs::kvData data;
  };
}

#endif // __FailInfo__FailList_h__
