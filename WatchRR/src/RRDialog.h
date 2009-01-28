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
#ifndef __WatchRR__RRDialog_h__
#define __WatchRR__RRDialog_h__

//#include <kvservice/qt/kvQtApp.h>
#include <KvApp.h>
#include <qdialog.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QCloseEvent>
#include <kvalobs/kvStation.h>
#include "RRTable.h"

class QPushButton;
class QStatusBar;
class QLineEdit;
class Q3BoxLayout;
class QCheckBox;

namespace kvalobs {
  class kvData;
}

namespace WatchRR
{
  class RRDialog
    : public QDialog
  {
    Q_OBJECT;

    void setup( RRTable * rrt ); 

  public:

    /**
     * \brief Will prompt user for a what data, and return the appropriate 
     *        RRDialog
     *
     * \parameter data represents the default values for station, obstime etc.
     */
    static RRDialog * getRRDialog( const kvalobs::kvData & data, std::list<kvalobs::kvStation>& slist, QWidget * parent );

    RRDialog( DayObsListPtr dol,	
	      const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter,
	      const QString & captionSuffix,
	      QWidget *parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags f = 0 );


    RRDialog( int station, const miutil::miDate date,
	      int type, int sensor, int level,
	      const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter,
	      QWidget *parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags f = 0 );
    virtual ~RRDialog( );

    const kvalobs::kvStation *getStation() const { return station; }

    void setReinserter( const kvalobs::DataReinserter<kvservice::KvApp> * ri )
    { dataReinserter = ri; }

  public:
    QLineEdit *stationInfo;
    RRTable *table;
    QPushButton *help;
    QPushButton * save;
    QPushButton *ok;
    QStatusBar *statusBar;
    Q3BoxLayout *mainLayout;

    //    QToolTipGroup *ttGroup;

  public slots:
    virtual void polish();

  protected:
    virtual void closeEvent( QCloseEvent * e );
    virtual void showEvent( QShowEvent * e );

    const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter;

  protected slots:
    virtual void reject();
    virtual void accept();

    bool saveData();

  private:
    QString captionSuffix_;
    const kvalobs::kvStation *station;
    bool shownFirstTime;
    std::list<kvalobs::kvStation> slist;
  };
}

#endif // __WatchRR__RRDialog_h__
