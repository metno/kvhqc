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
#ifndef __Weather__MultiStationSelection_h__
#define __Weather__MultiStationSelection_h__

#include <qdialog.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <vector>
#include <iterator>
#include <kvalobs/kvData.h>
#include <kvalobs/kvStation.h>
#include <decodeutility/DataReinserter.h>

class Q3ListView;
class Q3TabDialog;

namespace kvservice {
  class KvApp;
}

namespace Weather
{
  // defined in main.cc
  extern kvalobs::DataReinserter<kvservice::KvApp> * reinserter;

  class RRDialog;
  class StationSelection;

  class MultiStationSelection
    : public QDialog
  {
    Q_OBJECT;

    StationSelection * selector;
    Q3ListView * stations;
    Q3TabDialog* wTabs;
    std::list<kvalobs::kvStation> slist_;

  public:
    explicit MultiStationSelection( std::list<kvalobs::kvStation>& slist,
				    QWidget * parent = 0,
				    const kvalobs::kvData * data = 0 );
    virtual ~MultiStationSelection( );

  protected:
    virtual void keyPressEvent( QKeyEvent * e );


  public slots:
    void start();

  private slots:
    void doTransfer();
  };
}

#endif // __Weather__MultiStationSelection_h__
