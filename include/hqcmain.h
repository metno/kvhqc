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
#ifndef HQCMAIN_H
#define HQCMAIN_H

#include "connect2stinfosys.h"
#include "hqcdefs.h"
#include "textdatatable.h"

#include <decodeutility/DataReinserter.h>
#include <kvcpp/KvApp.h>
#include <qTimeseries/TSPlotDialog.h>

#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qmainwindow.h>

#include <memory>

class ClientButton;
class miMessage;
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
class ListDialog;
class ParameterDialog;
class RejectDialog;
class RejectTimeseriesDialog;
class TextDataDialog;
class TimeseriesDialog;

namespace model {
class KvalobsDataModel;
}
namespace Ui {
class HqcMainWindow;
}

/**
 * \brief The application's main window.
 */
class HqcMainWindow: public QMainWindow
{   Q_OBJECT

public:
    HqcMainWindow();
    ~HqcMainWindow();

    void startup();

  void makeObsDataList( kvservice::KvObsDataList& dataList );
  void makeTextDataList( kvservice::KvObsDataList& textdataList );

public Q_SLOTS:
  /*!
   * \brief Send observation times to Diana
   */
  void sendTimes();
  /*!
   * \brief Send message to show ground analysis in Diana
   */
  void sendAnalysisMessage();
  /*!
   * \brief Send station information to Diana
   */
  void sendStation(int);
  /*!
   * \brief Send complete observation at given time to Diana
   */
  void sendObservations(const timeutil::ptime& time, bool sendtime = true);
  /*!
   * \brief Send a parameter to Diana
   */
  void sendSelectedParam(const QString & param);
  /*!
   * \brief Reads the data and model_data tables in the kvalobs database
   *        and inserts the observations/model values in datalist/modeldatalist.
   */
  void readFromData(const timeutil::ptime&, const timeutil::ptime&, const std::vector<int>& stList);

public:

  /*!
   * \brief Returns true if the hour given as input is checked in the ClockDialog
   */
  bool timeFilter(int);
  /*!
   * \brief Returns true if the given typeId and environment corresponds
   *        to a station type checked in the ListDialog
   */
  bool hqcTypeFilter(int typeId, int environment, int stationId);
  bool typeIdFilter(int, int, int, const timeutil::ptime&, int);
  bool isAlreadyStored(const timeutil::ptime&, int);
  /*!
   * \brief
   */
  bool timeFilterChanged;

  /*!
   * \brief Reads the station table in the kvalobs database
   *        and inserts the station information in slist
   */
  void readFromStation();

    //! \brief Reads station info from stinfosys
    bool readFromStInfoSys();
    bool readFromStationFile();

  /*!
   * \brief Reads the parameter order from the file paramorder, then reaads the param
   *        table in the kvalobs database and inserts the station information in parmap
   */
  void readFromParam();
  /*!
   * \brief Reads the typeids file to find which typeids to show
   */
  //  void readFromTypeIdFile();
  void checkTypeId(int);
  /*!
   * \brief Reads the obs_pgm table in the kvalobs database and
   *       inserts the station information in obsPgmList and statList
   */
  void readFromObsPgm();
  /*!
   * \brief Retrieves from stlist the position and height of a given station
   */
  void findStationPos(int, double&, double&, double&);
  /*!
   * \brief Retrieves from stlist information about a given station
   */
  void findStationInfo(int, QString&, double&, double&, double&, int&, int&);
  /*!
   * \brief A primitive horizontal tiling of the errorhead and errorlist windows
   */
  void tileHorizontal();
  /*!
   * \brief Extract the typeid from the obspgmlist for a given station, parameter and obstime
   */
  int findTypeId(int, int, int, const timeutil::ptime&);
  //  int findTypeId(int, int, const timeutil::ptime&);
  /*!
   * \brief Some parameters are displayed in Diana with other units than those in the kvalobs database.
   *        This method converts from "Kvalobs" to "Diana" units
   */
  double dianaValue(int, bool, double, double);
  /*!
   * \brief Some parameters are displayed in Diana with other names than those in the kvalobs database.
   *        This method converts from "Kvalobs" to "Diana" names.
   */
  std::string dianaName(const std::string&);
  /*!
   * \brief From given typeId and environment is generated a text string to be sent to Diana
   */
  std::string hqcType(int, int);

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

    const std::list<listStat_t>& getStationDetails();
    const ObsTypeList& getObsTypeList() { return otpList; }

public Q_SLOTS:

  void saveDataToKvalobs(const kvalobs::kvData& toSave);

/*!
 * \brief Produces the data table or the error list
*/
  void ListOK();

/*!
 * \brief Called when OK button in ClockDialog is clicked
*/
  void ClkOK();
/*!
 * \brief Called when OK button in DianaShowDialog is clicked.
 *        Initializes the maps with the parameters to be shown in Diana.
*/
  void dianaShowOK();
/*!
 * \brief Produces time series plots
*/
  void TimeseriesOK();
/*!
 * \brief Called when OK button in ParameterDialog is clicked
*/
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
  bool firstObs;
  int sLevel;

  /// Paramid to parameter name
  QMap<int,QString> parMap;

  /// True after first time ListOk() have been invoked with valid input
  bool listExist;

  QString wElement;
  QString userName;

  /// The parameters that the user have selected
  QStringList selPar;

  bool tsVisible;

    std::auto_ptr<Ui::HqcMainWindow> ui;
  ClientButton* pluginB;
  bool dianaconnected;

    QTimer* mVersionCheckTimer;
    HintWidget* mHints;

  ObsTypeList otpList;
  std::list<kvalobs::kvObsPgm> obsPgmList;
  std::list<kvalobs::kvStation> slist;
  std::list<kvalobs::kvParam> plist;

  /**
   * Parameters from parameter groups. The keys will be the user's
   * presentation strings, and not the strings in the config file
   *
   * @todo make bindings between menu elements and config file dynamic
   */
  QMap<QString, std::vector<int> > parameterGroups;

  TSPlotDialog* tspdialog; // timeseries-plot

  timeutil::ptime dianaTime;
  typedef QMap<std::string,std::string> NameMap;
  NameMap dnMap;
  typedef QMap<std::string,bool> ModelMap;
  ModelMap mdMap;
  typedef QMap<std::string,bool> DiffMap;
  DiffMap diMap;
  typedef QMap<std::string,bool> PropMap;
  PropMap prMap;

  /**
   * Settings
   */
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

protected:
  // socket methods
  void initDiana();
  void sendMessage(miMessage&);
  void readErrorsFromqaBase(int&, int&);
  void showWindow(QWidget* w);

    std::list<listStat_t> listStat;
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

  // socket slots
#ifdef METLIBS_BEFORE_4_9_5
  void processLetterOld(miMessage&);
#endif
  void processLetter(const miMessage&);
  void processConnect();
  void cleanConnection();

  void errListMenu();
  void allListMenu();
  void errLogMenu();
  void dataListMenu();
  void errLisaMenu();
  //  void textDataMenu();
  void rejectedMenu();
  void textDataMenu();
  void rejectedOK();
  void textDataOK();

  /**
   * Bring up a the WatchRR specification dialog
   */
  void showWatchRR();
  /**
   * Bring up a the WatchWeather specification dialog
   */
  void showWeather();

  void listMenu();
  void clockMenu();
  void dianaShowMenu();
  void paramMenu();
  void timeseriesMenu();

  void updateSaveFunction( QMdiSubWindow * w );

Q_SIGNALS:
  void statTimeReceived(const QString&);
  void newStationList(std::vector<QString>&);
  void newParameterList(const QStringList&);
  void saveData();
  void windowClose();
  void printErrorList();

  /**
   * \brief Emitted when a new station and/or obstime has been selected in the
   *        errorlist.
   */
  void errorListStationSelected(int station, const timeutil::ptime& obstime);
};

/**
 * \brief Get o's owning HqcMainWindow, or NULL if there is none.
 */
HqcMainWindow * getHqcMainWindow( const QObject * o );
HqcMainWindow * getHqcMainWindow( QObject * o );

#endif
