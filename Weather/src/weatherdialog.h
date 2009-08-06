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
#ifndef __Weather__WeatherDialog_h__
#define __Weather__WeatherDialog_h__

#include <kvcpp/KvApp.h>
#include <QDialog>
#include <kvalobs/kvStation.h>
#include "weathertable.h"
#include "timeobs.h"
#include "dataconsistencyverifier.h"
#include <decodeutility/DataReinserter.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvStation.h>

using namespace std;
using namespace kvservice;

class QPushButton;
class QStatusBar;
class QLineEdit;
class Q3BoxLayout;
class QCheckBox;
class QTabWidget;

const int params[] = { 211,214,216,213,215,262,178,173,177,1,61,81,86,87,83,90,15,14,55,108,
		       109,110,112,18,7,273,41,31,32,33,42,43,34,36,38,40,
		       23,24,22,403,404,131,134,151,154,250,221,9,12};
const int NP = 49;
const int NC = 5;

struct SynObs {
  int stnr;
  int snr;
  miutil::miTime otime;
  int typeId[NP];
  int sensor[NP];
  double orig[NP];
  double corr[NP];
  string controlinfo[NP];
};

namespace kvalobs {
  class kvData;
}
typedef list<kvalobs::kvData>::iterator                   IDataList;

namespace Weather
{
  class WeatherDialog
    : public QDialog
      //    : public Q3TabDialog
  {
    friend class WeatherTable;
    Q_OBJECT;

  public:

    /**
     * \brief Will prompt user for a what data, and return the appropriate
     *        WeatherDialog
     *
     * \parameter data represents the default values for station, obstime etc.
     */
/**
 * \brief Get o's owning WeatherDialog, or NULL if there is none.
 */
    static WeatherDialog * getWeatherDialog( const kvalobs::kvData & data, std::list<kvalobs::kvStation>& slist, QWidget * parent, Qt::WindowFlags f );

    WeatherDialog( TimeObsListPtr dol, int type, int sensor,
	      const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter,
		   QWidget *parent = 0, const char* name = 0, bool modal = FALSE );


    WeatherDialog( int station, const miutil::miTime clock, int type, int sensor,
	      const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter,
		   QWidget *parent = 0, const char* name = 0, bool modal = FALSE);

    virtual ~WeatherDialog( );
    /**
     * \brief [Start, stop) dates for which to fetch data.
     */
    typedef std::pair<miutil::miTime, miutil::miTime> DateRange;

    /**
     * \brief Date range for editor
     */
    const DateRange & getDateRange() const { return dateRange; }

    QTabWidget* tabWidget;

    const kvalobs::kvStation *getStation() const { return station; }
    int getStationId() const { return stationId; }

    void setReinserter( const kvalobs::DataReinserter<kvservice::KvApp> * ri )
    { dataReinserter = ri; }

    bool paramInParamsList(int);
    bool typeFilter(int, int);
    bool sensorFilter(int, int);
    typedef QPair<float,float> oldNewPair;
    std::vector<oldNewPair> oldNew;

  public:
    QPushButton *help;
    QPushButton * save;
    QPushButton *ok;
    QStatusBar *statusBar;
    Q3BoxLayout *mainLayout;
    SynObs synObs;
    typedef vector<SynObs> SynObsList;
    SynObsList synObsList;
    //    QToolTipGroup *ttGroup;
    KvObsDataList ldList;

  public slots:
    //    virtual void polish();
    bool saveData();

signals:
    void dontStore();

  protected:
    const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter;
    TimeObsListPtr observations;
    bool saveData(const kvalobs::DataReinserter<kvservice::KvApp> *);

  private:
    QMap<int, int> parameterIndex;
    const kvalobs::kvStation *station;
    int stationId;
    DateRange dateRange;
    bool shownFirstTime;
    void setupOrigTab( SynObsList&, int, QTabWidget* );
    void setupCorrTab( SynObsList&, int, QTabWidget* );
    void setupFlagTab( SynObsList&, int, QTabWidget* );
    void setupStationInfo();
    WeatherTable* cTab;
  };
}

#endif // __Weather__WeatherDialog_h__
