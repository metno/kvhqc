/* -*- c++ -*-
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

#include "weathertable.h"
#include "timeobs.h"
#include "timeutil.hh"
#include "dataconsistencyverifier.h"

#include <decodeutility/DataReinserter.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvStation.h>
#include <kvcpp/KvApp.h>

#include <QtGui/QDialog>

#include <string>

class QPushButton;
class QStatusBar;
class QLineEdit;
class Q3BoxLayout;
class QCheckBox;
class QTabWidget;

struct SynObs {
  int stnr;
  int snr;
  timeutil::ptime otime;
  int typeId[Weather::NP];
  int sensor[Weather::NP];
  double orig[Weather::NP];
  double corr[Weather::NP];
  std::string controlinfo[Weather::NP];
};

typedef std::list<kvalobs::kvData>::iterator                   IDataList;
typedef std::list<kvalobs::kvObsPgm>                          ObsPgmList;
typedef std::list<kvalobs::kvObsPgm>::const_iterator        CIObsPgmList;
typedef std::list<int>                                          OpgmList;
typedef std::list<int>::iterator                               IOpgmList;

namespace Weather
{

class WeatherDialog : public QDialog
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

    WeatherDialog(TimeObsListPtr dol, int type, int sensor,
                  const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter,
                  QWidget *parent = 0);
    
    WeatherDialog(int station, const timeutil::ptime& clock, int type, int sensor,
                  const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter,
                  QWidget *parent = 0);
    
    virtual ~WeatherDialog( );

    void initData(const timeutil::ptime& clock, int type, int sensor);
    void setupGUI(int type);

    /**
     * \brief [Start, stop) dates for which to fetch data.
     */
    typedef std::pair<timeutil::ptime, timeutil::ptime> DateRange;

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
    typedef std::vector<SynObs> SynObsList;
    SynObsList synObsList;
    //    QToolTipGroup *ttGroup;
    kvservice::KvObsDataList ldList;

  public slots:
    //    virtual void polish();
    bool saveData();

signals:
    void dontStore();

  protected:
    const kvalobs::DataReinserter<kvservice::KvApp> * dataReinserter;
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
    void opgmList( OpgmList& opgtl, const kvalobs::kvObsPgm& op);
    WeatherTable* cTab;
    std::list<kvalobs::kvObsPgm> obsPgmList;
    std::list<long> statList;

  };
}

#endif // __Weather__WeatherDialog_h__
