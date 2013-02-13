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

#include "connect2stinfosys.h"
#include "hqcdefs.h"
#include "textdatatable.h"

#include <decodeutility/DataReinserter.h>
#include <kvcpp/KvApp.h>

#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qmainwindow.h>

#include <memory>

class ClientButton;
class miMessage;
class TSPlotDialog;
QT_BEGIN_NAMESPACE
class QAction;
class QMdiArea;
class QMdiSubWindow;
class QTimer;
QT_END_NAMESPACE

class AcceptTimeseriesDialog;
class ClockDialog;
class DataTable;
class DianaShowDialog;
class HintWidget;
class HqcDianaHelper;
class ListDialog;
class ParameterDialog;
class RejectDialog;
class RejectTimeseriesDialog;
class TextDataDialog;
class TimeseriesDialog;
class KvalobsAccess;
class KvalobsModelAccess;

namespace model {
class KvalobsDataModel;
}
namespace Ui {
class HqcMainWindow;
}

class HqcMainWindow: public QMainWindow
{   Q_OBJECT

public:
    HqcMainWindow();
    ~HqcMainWindow();

    void startup();

    void makeObsDataList( kvservice::KvObsDataList& dataList);
    void makeTextDataList( kvservice::KvObsDataList& textdataList);

public Q_SLOTS:
    //! send all observation times to Diana
    void sendTimes();

public:
    /*!
     * \brief Returns true if the hour given as input is checked in the ClockDialog
     */
    bool timeFilter(int);

    //! true if the given typeId and environment corresponds to a station type checked in the ListDialog
    bool hqcTypeFilter(const QSet<QString>& selectedStationTypes, int typeId, int environment);
    bool typeIdFilter(int, int, int, const timeutil::ptime&, int);
    bool isAlreadyStored(const timeutil::ptime&, int);

    bool timeFilterChanged;

    void readFromStation();

    //! reads station info from stinfosys
    bool readFromStInfoSys();
    bool readFromStationFile();

    /*! \brief Reads the parameter order from the file paramorder, then reaads the param
     *        table in the kvalobs database and inserts the station information in parmap */
    void readFromParam();

    //! read data and model_data from kvalobs to datalist/modeldatalist
    void readFromData(const timeutil::ptime&, const timeutil::ptime&, const std::vector<int>& stList);

    void checkTypeId(int);

    //! Retrieves from stlist the position and height of a given station
    void findStationPos(int, double&, double&, double&);

    //! Retrieves from stlist information about a given station
    void findStationInfo(int, QString&, double&, double&, double&, int&, int&);

    //! A primitive horizontal tiling of the errorhead and errorlist windows
    void tileHorizontal();

    //! Extract the typeid from the obspgmlist for a given station, parameter and obstime
    int findTypeId(int, int, int, const timeutil::ptime&);

    ListDialog* lstdlg;
    ClockDialog* clkdlg;
    DianaShowDialog* dshdlg;
    ParameterDialog* pardlg;
    TextDataDialog* txtdlg;
    RejectDialog* rejdlg;
    std::vector<kvalobs::kvRejectdecode> rejList;
    TimeseriesDialog* tsdlg;
    RejectTimeseriesDialog* rjtsdlg;
    AcceptTimeseriesDialog* actsdlg;

    model::KvalobsDataListPtr datalist;
    std::vector<modDatl> modeldatalist;

    /// This holds the value of the previously used stList
    std::vector<TxtDat> txtList;
    listType lity;
    int selParNo[NOPARAMALL];
    std::vector<currentType> currentTypeList;
    kvalobs::DataReinserter<kvservice::KvApp> *reinserter;
    /// True if all types have been selected, as opposed to prioritized parameters
    bool isShTy;

    const listStat_l& getStationDetails();

private Q_SLOTS:
    void navigateTo(const kvalobs::kvData& d);
    void receivedStationFromDiana(int stationid);
    void receivedTimeFromDiana(const timeutil::ptime& time);

    void saveDataToKvalobs(const kvalobs::kvData& toSave);

    //! Produces the data table or the error list
    void ListOK();

    //! Called when OK button in ClockDialog is clicked
    void ClkOK();

    //! Called when OK button in DianaShowDialog is clicked.
    /*! Initializes the maps with the parameters to be shown in Diana. */
    void dianaShowOK();

    //! Produces time series plots
    void TimeseriesOK();

    //! Called when OK button in ParameterDialog is clicked
    void paramOK();

private Q_SLOTS:
    void showFlags();
    void showOrigs();
    void showMod();
    void showStat();
    void showPos();
    void showHeight();
    void showTyp();
    void airPress();
    void temperature();
    void precipitation();
    void visuals();
    void sea();
    void synop();
    void climateStatistics();
    void priority();
    void wind();
    void plu();
    void all();
    void clk();
    void dsh();
    void rejectTimeseries();
    void rejectTimeseriesOK();
    void acceptTimeseries();
    void acceptTimeseriesOK();
    void startKro();
    void screenshot();

    void onVersionCheckTimeout();

private:
    void selectParameterGroup(const QString& group);
    void exitNoKvalobs();

private:
    model::KvalobsDataModel * dataModel;
    int sLevel;

    /// True after first time ListOk() have been invoked with valid input
    bool listExist;

    QString wElement;
    QString userName;

    /// The parameters that the user have selected
    QStringList selPar;

    bool tsVisible;

    std::auto_ptr<Ui::HqcMainWindow> ui;
    ClientButton* pluginB;
    std::auto_ptr<HqcDianaHelper> mDianaHelper;

    QTimer* mVersionCheckTimer;
    HintWidget* mHints;

    /**
     * Parameters from parameter groups. The keys will be the user's
     * presentation strings, and not the strings in the config file
     *
     * @todo make bindings between menu elements and config file dynamic
     */
    QMap<QString, std::vector<int> > parameterGroups;

    TSPlotDialog* tspdialog; // timeseries-plot

    void closeEvent(QCloseEvent* event);
    void writeSettings();
    void readSettings();
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

protected:
    listStat_l listStat;
    timeutil::ptime mLastStationListUpdate;

    void moveEvent(QMoveEvent* event);
    void resizeEvent(QResizeEvent* event);

private Q_SLOTS:
    void closeWindow();
    void helpUse();
    void helpFlag();
    void helpParam();
    void about();
    void aboutQt();

    void errListMenu();
    void allListMenu();
    void errLogMenu();
    void dataListMenu();
    void errLisaMenu();
    void rejectedMenu();
    void textDataMenu();
    void rejectedOK();
    void textDataOK();

    //! bring up the WatchRR dialog
    void showWatchRR();

    //! bring up the WatchWeather dialog
    void showWeather();

    void listMenu();
    void clockMenu();
    void dianaShowMenu();
    void paramMenu();
    void timeseriesMenu();

    void updateSaveFunction(QMdiSubWindow * w);

Q_SIGNALS:
    void statTimeReceived(int stationid, const timeutil::ptime& obstime, int typeID);
    void timeReceived(const timeutil::ptime& obstime);

    void newStationList(std::vector<QString>&);
    void newParameterList(const QStringList&);
    void saveData();
    void windowClose();
    void printErrorList();
};

//! Get o's owning HqcMainWindow, or NULL if there is none.
HqcMainWindow * getHqcMainWindow(const QObject* o);
HqcMainWindow * getHqcMainWindow(QObject* o);

#endif
