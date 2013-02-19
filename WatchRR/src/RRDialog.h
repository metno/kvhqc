/* -*- c++ -*-

HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2013 met.no

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

#include "RRTable.h"
#include <kvcpp/KvApp.h>
#include <QtGui/QDialog>
#include <QtGui/QShowEvent>
#include <QtGui/QCloseEvent>

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

class RRDialog : public QDialog
{ Q_OBJECT;
    
public:
    /**
     * \brief Will prompt user for a what data, and return the appropriate
     *        RRDialog
     *
     * \parameter data represents the default values for station, obstime etc.
     */
    static RRDialog * getRRDialog( const kvalobs::kvData & data, QWidget* parent, Qt::WindowFlags f );
    
    RRDialog( DayObsListPtr dol,
	      const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter,
	      const QString & captionSuffix,
	      QWidget *parent = 0, const char* name = 0, bool modal = FALSE );


    RRDialog( int station, const timeutil::pdate& date,
	      int type, int sensor, int level,
	      const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter,
	      QWidget *parent = 0, const char* name = 0, bool modal = FALSE );

    ~RRDialog();

    void setReinserter(const kvalobs::DataReinserter<kvservice::KvApp> * ri)
        { dataReinserter = ri; }

    RRTable *table;

public Q_SLOTS:
    void polish();
    
protected:
    void closeEvent(QCloseEvent * e);
    void showEvent(QShowEvent * e);
    
protected Q_SLOTS:
    virtual void reject();
    virtual void accept();
    
private:
    void setup( RRTable * rrt );
    bool saveData();

private:
    QLineEdit *stationInfo;
    QPushButton *help;
    QPushButton * save;
    QPushButton *ok;
    QStatusBar *statusBar;
    Q3BoxLayout *mainLayout;

    const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter;

    QString captionSuffix_;
    int mStationId;
    bool shownFirstTime;
    std::list<kvalobs::kvStation> slist;
};

} // namespace WatchRR

#endif // __WatchRR__RRDialog_h__
