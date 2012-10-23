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
#ifndef __WatchRR__StationSelection_h__
#define __WatchRR__StationSelection_h__

#include "timeutil.hh"
#include <QtGui/qwidget.h>
#include <map>

class QLineEdit;
class Q3DateEdit;
namespace kvalobs {
  class kvData;
}

namespace WatchRR
{
  class StationSelection
    : public QWidget
  {
    Q_OBJECT;
  public:
    StationSelection( QWidget * parent, const kvalobs::kvData * data = 0 );

    kvalobs::kvData getKvData() const;

    int station() const;
    timeutil::pdate obstime() const;
    int typeID() const;
    int sensor() const;
    int level() const;

  private slots:
    void updateTypeID_();
    void setupTypeFromStation_();

  private:
    QLineEdit * station_;
    Q3DateEdit * obstime_;
    QLineEdit * typeID_;
    QLineEdit * sensor_;
    QLineEdit * level_;

    typedef std::map<int,int> TypeFromStation;
    static TypeFromStation typeFromStation_;
  };
}

#endif // __WatchRR__StationSelection_h__
