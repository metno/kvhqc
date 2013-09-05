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
#ifndef HQCMAIN_H
#define HQCMAIN_H

#include "KvalobsData.h"
#include "StInfoSysBuffer.hh"
#include "hqcdefs.h"

#include <QtCore/qset.h>
#include <QtCore/qstring.h>
#include <QtGui/qmainwindow.h>

#include <memory>

class ClientButton;
class HelpDialog;
class miMessage;
class TSPlotDialog;
QT_BEGIN_NAMESPACE
class QAction;
class QMdiArea;
class QMdiSubWindow;
class QTimer;
QT_END_NAMESPACE

class AcceptTimeseriesDialog;
class DataTable;
class DianaShowDialog;
class HintWidget;
class HqcDianaHelper;
class KvalobsAccess;
class KvalobsModelAccess;
class ListDialog;
class RejectDialog;
class RejectTimeseriesDialog;
class SensorTime;
class TextDataDialog;
class TimeseriesDialog;

namespace model {
class KvalobsDataModel;
}
namespace Ui {
class HqcMainWindow;
}

class HqcMainWindow: public QMainWindow
{ Q_OBJECT;

public:
    HqcMainWindow();
    ~HqcMainWindow();

    void startup();

    void makeObsDataList(kvservice::KvObsDataList& dataList);

    bool saveDataToKvalobs(std::list<kvalobs::kvData>& toSave);
    bool saveDataToKvalobs(kvalobs::kvData& toSave1);

    //! Extract the typeid from the obspgmlist for a given station, parameter and obstime
    int findTypeId(int, int, int, const timeutil::ptime&);

    const listStat_l& getStationDetails();

    HqcReinserter* getReinserter()
        { return reinserter; }

    void setReinserter(HqcReinserter* r, const QString& userName);

public Q_SLOTS:
    //! send all observation times to Diana
    void sendTimes();

protected:
    void moveEvent(QMoveEvent* event);
    void resizeEvent(QResizeEvent* event);

Q_SIGNALS:
    void statTimeReceived(int stationid, const timeutil::ptime& obstime, int typeID);
    void timeReceived(const timeutil::ptime& obstime);

    void newStationList(std::vector<QString>&);
    void newParameterList(const std::vector<int>&);
    void saveData();
    void windowClose();
    void printErrorList();

private Q_SLOTS:
    void navigateTo(const SensorTime& st);
    void receivedStationFromDiana(int stationid);
    void receivedTimeFromDiana(const timeutil::ptime& time);

    void errListMenu();
    void allListMenu();
    void errLogMenu();
    void dataListMenu();
    void errLisaMenu();
    void allListSalenMenu();
    void ListOK();
    void rejectedOK();
    void textDataOK();
    void dianaShowOK();
    void TimeseriesOK();
    void rejectTimeseriesOK();
    void acceptTimeseriesOK();

    void startKro();
    void screenshot();
    void helpUse();
    void helpNews();
    void helpFlag();
    void helpParam();
    void about();
    void aboutQt();

    void closeWindow();

    //! bring up the WatchRR dialog
    void showWatchRR();

    //! bring up the WatchWeather dialog
    void showWeather();

    void onVersionCheckTimeout();

private:
    //! read data and model_data from kvalobs to datalist/modeldatalist
    void readFromData(const timeutil::ptime&, const timeutil::ptime&, const std::vector<int>& stList);

    //! Retrieves from stlist information about a given station
    void findStationInfo(int, QString&, double&, double&, double&, int&, int&);

    //! Retrieves from stlist the position and height of a given station
    void findStationPos(int, double&, double&, double&);

    void checkTypeId(int);
    bool typeIdFilter(int, int, int, const timeutil::ptime&, int);

    //! true if the given typeId and environment corresponds to a station type checked in the ListDialog
    bool hqcTypeFilter(const QSet<QString>& selectedStationTypes, int typeId, int environment);

    //! true if the hour given as input is checked in the ListDialog
    bool timeFilter(int);

    void listMenu(listType lt);
    void exitNoKvalobs();
    void closeEvent(QCloseEvent* event);
    void writeSettings();
    void readSettings();
    void checkVersionSettings();

    void readFromStation();

    /*! \brief Reads the parameter order from the file paramorder, then reaads the param
     *        table in the kvalobs database and inserts the station information in parmap */
    void readFromParam();

    //! A primitive horizontal tiling of the errorhead and errorlist windows
    void tileHorizontal();

    bool keepDataInList(const kvalobs::kvData& kvd, int absTypeId, int env,
                        const QSet<QString> selectedStationTypes,
                        const bool allStationTypes, const bool showPrioritized);
    void putToDataList(const kvalobs::kvData& kvd);

private:
    ListDialog* lstdlg;
    DianaShowDialog* dshdlg;
    TextDataDialog* txtdlg;
    RejectDialog* rejdlg;
    std::vector<kvalobs::kvRejectdecode> rejList;
    TimeseriesDialog* tsdlg;
    RejectTimeseriesDialog* rjtsdlg;
    AcceptTimeseriesDialog* actsdlg;
    HelpDialog* mHelpDialog;

    model::KvalobsDataListPtr datalist;
    std::vector<modDatl> modeldatalist;

    listType lity;
    std::vector<currentType> currentTypeList;
    HqcReinserter* reinserter;

    model::KvalobsDataModel * dataModel;
    int sLevel;

    /// True after first time ListOk() have been invoked with valid input
    bool listExist;

    QString userName;

    std::vector<int> mSelectedParameters;
    std::set<int> mSelectedTimes;

    std::auto_ptr<Ui::HqcMainWindow> ui;
    ClientButton* pluginB;
    std::auto_ptr<HqcDianaHelper> mDianaHelper;

    QTimer* mVersionCheckTimer;
    HintWidget* mHints;

    TSPlotDialog* tspdialog; // timeseries-plot

    int parFind;

    struct Param
    {
        bool item;
        QString text;
        bool mark;
        bool noMark;
        bool all;
    };

    boost::shared_ptr<KvalobsAccess> kda;
    boost::shared_ptr<KvalobsModelAccess> kma;
};

//! Get o's owning HqcMainWindow, or NULL if there is none.
HqcMainWindow * getHqcMainWindow(const QObject* o);
HqcMainWindow * getHqcMainWindow(QObject* o);

#endif
