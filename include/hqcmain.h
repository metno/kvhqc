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
#ifndef HQCMAIN_H
#define HQCMAIN_H

#include "ListDialog.h"
#include "ClockDialog.h"
#include "dianashowdialog.h"
#include "TimeseriesDialog.h"
#include "parameterdialog.h"
#include "textdatadialog.h"
#include "textdatatable.h"
#include "rejectdialog.h"
#include "rejecttable.h"
#include "errorlist.h"
#include <qwidget.h>
#include <fstream>
#include <iostream>
#include <kvcpp/KvApp.h>
#include <qmainwindow.h>
#include <qobject.h>
//#include <q3process.h>
#include <qmenubar.h> 
#include <qmap.h>
//#include <q3multilineedit.h>
#include <qpoint.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qpushbutton.h>
//#include <q3accel.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qobject.h>
//#include <q3vbox.h>
//#include <q3table.h>
#include <QToolBar>
#include <qtoolbutton.h>
#include <qicon.h>
#include <qcursor.h>

#include <qUtilities/ClientButton.h>

#include <puTools/miString.h>
#include <qUtilities/miMessage.h>
#include <decodeutility/DataReinserter.h>

#include <qTimeseries/TSPlotDialog.h>
#include "hqcdefs.h"

class QAction;
class QMdiArea;
class QMdiSubWindow;

using namespace std;
using namespace kvalobs;
using namespace kvservice;
class DataTable;


/**
 * \brief Get o's owning HqcMainWindow, or NULL if there is none.
 */
HqcMainWindow * getHqcMainWindow( const QObject * o );
HqcMainWindow * getHqcMainWindow( QObject * o );


/**
 * \brief The application's main window.
 */
class HqcMainWindow: public QMainWindow
{
  Q_OBJECT
public:

  HqcMainWindow();
  ~HqcMainWindow();
  void makeObsDataList( kvservice::KvObsDataList& dataList );
  void makeTextDataList( kvservice::KvObsDataList& textdataList );
  int nuroprpar;
  int nucoprpar;


public slots:
  /*!
   * \brief Send observation times to Diana 
   */
  void sendTimes();
  /*!
   * \brief Send message to show ground analysis in Diana
   */
  bool sendAnalysisMessage();
  /*!
   * \brief Send station information to Diana
   */
  void sendStation(int);
  /*!
   * \brief Send complete observation at given time to Diana
   */
  void sendObservations(miutil::miTime,bool =true);
  /*!
   * \brief Send a parameter to Diana
   */
  void sendSelectedParam(const QString & param);
  /*!
   * \brief When a parameter value is changed, the new value is 
   *        sent to the datalist, to the timeseries and to Diana.
   */
  void updateParams(int, 
		    miutil::miTime,
		    miutil::miString, 
		    miutil::miString, 
		    miutil::miString);
  /*!
   * \brief The boolean variable kvBaseIsUpdated is set to FALSE at the 
   *        start, and to TRUE when an update is sent to the database.
   */
  void setKvBaseUpdated(bool);
  /*!
   * \brief Returns the value of kvBaseIsUpdated  
   */
  bool kvBaseUpdated() {return kvBaseIsUpdated;};
  /*!
   * \brief Reads the data table in the kvalobs database
   *        and inserts the observations in datalist.
   */
  void readFromData(const miutil::miTime&, const miutil::miTime&, listType);
  /*!
   * \brief Reads the modeldata table in the kvalobs database
   *        and inserts the observations in modeldatalist.
   */
  void readFromModelData(const miutil::miTime&, const miutil::miTime&);
  /*!
   * \brief Extracts all the data for one station and one time from datalist
   */
  void listData(int, 
		int&, 
		miutil::miTime&, 
		double*, 
		int*, 
		double*, 
		double*,
		string*,
		string*,
		string*,
		int*,
		int&,
		int&);

public:

  /*!
   * \brief Returns true if the hour given as input is checked in the ClockDialog
   */
  bool timeFilter(int);
  /*!
   * \brief Returns true if the given typeId and environment corresponds
   *        to a station type checked in the ListDialog 
   */
  bool hqcTypeFilter(const int&, int, int);
  bool typeIdFilter(int, int, int, miutil::miTime, int);
  bool isAlreadyStored(miutil::miTime, int);
  /*!
   * \brief 
   */
  bool timeFilterChanged;
  /*!
   * \brief Reads the station table in the kvalobs database
   *        and inserts the station information in slist
   */
  void readFromStation();
  /*!
   * \brief 
   */
  void readFromStationFile(int);
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
  int findTypeId(int, int, int, miutil::miTime);
  //  int findTypeId(int, int, miutil::miTime);
  /*!
   * \brief Some parameters are displayed in Diana with other units than those in the kvalobs database.
   *        This method converts from "Kvalobs" to "Diana" units
   */
  double dianaValue(int, bool, double, double);
  /*!
   * \brief Some parameters are displayed in Diana with other names than those in the kvalobs database.
   *        This method converts from "Kvalobs" to "Diana" names.
   */
  miutil::miString dianaName(miutil::miString);
  /*!
   * \brief From given typeId and environment is generated a text string to be sent to Diana
   */
  miutil::miString hqcType(int, int);

  ListDialog* lstdlg;
  ClockDialog* clkdlg;
  DianaShowDialog* dshdlg;
  ParameterDialog* pardlg;
  TextDataDialog* txtdlg;
  RejectDialog* rejdlg;
  Rejects* rejects;
  vector<kvalobs::kvRejectdecode> rejList;
  TimeseriesDialog* tsdlg;
  model::KvalobsDataListPtr datalist;
  vector<modDatl> modeldatalist;

  /// List of selected stations
  vector<int> stList;

  vector<int> stnrList;

  /// This holds the value of the previously used stList
  vector<int> remstList;
  vector<TxtDat> txtList;
  listType lity;
  mettType metty;
  int selParNo[NOPARAMALL];
  currentType crT;
  vector<currentType> currentTypeList;
  vector<QString> statLineList;
  DataReinserter<kvservice::KvApp> *reinserter;

public slots:

  void saveDataToKvalobs(const kvalobs::kvData & toSave);

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
/*!
 * \brief Called when stationSelect button in ListDialog is clicked
*/
  void stationOK();

private slots:
  void showFlags();
  void showOrigs();
  void showMod();
  void showStat();
  void showPos();
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
  void startKro();
private:
  bool firstObs;
  int sLevel;
  int sSensor;

  /// Paramid to parameter name
  QMap<int,QString> parMap;

  /// True after first time ListOk() have been invoked with valid input
  bool listExist;

  bool kvBaseIsUpdated;
  QString wElement;
  QString userName;

  /// The parameters that the user have selected
  QStringList selPar;

  /// User selection whether to display flags in data list
  QAction * flID;

  /// User selection whether to display original values in data list
  QAction * orID;

  /// User selection whether to display model data in data list
  QAction * moID;

  /// User selection whether to display station's name
  QAction * stID;

  /// User selection whether to display station's location
  QAction * poID;

  QAction * tyID;
  QAction * apID;
  QAction * taID;
  //  QAction * otID;
  //  QAction * huID;
  QAction * wiID;
  QAction * prID;
  QAction * clID;
  QAction * seID;
  QAction * syID;
  QAction * klID;
  QAction * piID;
  QAction * plID;
  QAction * alID;

  /// True if all types have been selected, as opposed to prioritized parameters
  bool isShTy;
  int synopType;
  int autoobsType;
  int kvalobsType;
  int hqcFrom;
  int hqcTo;
  bool isSynop;
  bool isAuto;
  bool isKvalobs;
  bool tsVisible;
  ClientButton* pluginB;
  //  DianaConnection* diaCon;


//  Q3PopupMenu* file;
  QAction * saveAction;
  QAction * printAction;
//  //KTEST
  int filePrintMenuItem;
  QMenu* choice;
  QMenu* showmenu;
  QMenu* weathermenu;
  QMenu* clockmenu;
  QMenu* typeIdmenu;



  QMdiArea * ws;
  //  QPainter* logo;
  //  void paintEvent(QPaintEvent*);
  // socket variables
  bool usesocket;
  bool dianaconnected;
  QString dateandTime;
  QString kvParam[NOPARAMALL];
  QString kdbParam[NOPARAMALL];

  //  DataList dlist;
  ModelDataList mdlist;
  ObsTypeList otpList;
  std::list<kvalobs::kvObsPgm> obsPgmList;
  std::list<kvalobs::kvStation> sillist;
  std::list<kvalobs::kvStation> slist;
  std::list<kvalobs::kvParam> plist;
  std::list<long> statList;



#warning sizes here must not be hardcoded!!
  //int order[NOPARAMALL];
  std::vector<int> order;

  /**
   * Parameters from parameter groups. The keys will be the user's
   * presentation strings, and not the strings in the config file
   *
   * @todo make bindings between menu elements and config file dynamic
   */
  QMap<QString, std::vector<int> > parameterGroups;

//  int airPressOrder[NOPARAMAIRPRESS];
//  int tempOrder[NOPARAMTEMP];
//  int precOrder[NOPARAMPREC];
//  int visualOrder[NOPARAMVISUAL];
//  int waveOrder[NOPARAMWAVE];
//  int synopOrder[NOPARAMSYNOP];
//  int klstatOrder[NOPARAMKLSTAT];
//  int priorityOrder[NOPARAMPRIORITY];
//  int windOrder[NOPARAMWIND];
//  int pluOrder[NOPARAMPLU];
  
  TSPlotDialog* tspdialog; // timeseries-plot
  StationTable* stationTable;

  /// Station selection dialog
  StationSelection* statSelect;

  miutil::miTime dianaObsTime;
  miutil::miTime dianaTime;
  typedef QMap<miutil::miString,miutil::miString> NameMap;
  NameMap dnMap;
  typedef QMap<miutil::miString,bool> ModelMap;
  ModelMap mdMap;
  typedef QMap<miutil::miString,bool> DiffMap;
  DiffMap diMap;
  typedef QMap<miutil::miString,bool> PropMap;
  PropMap prMap;


protected:
  // socket methods
  void initDiana();
  void sendMessage(miMessage&);
  void sendImage(const miutil::miString name, const QImage& image);
  //  void sendShowText(const miString site);
  void readErrorsFromqaBase(int&, int&);
  void showWindow(QWidget* w);

QStringList listStatName;
QStringList listStatNum;
QStringList listStatHoh;
QStringList listStatType;
QStringList listStatFylke;
QStringList listStatKommune;
QStringList listStatWeb;
QStringList listStatPri;
QStringList listStatFromTime;
QStringList listStatToTime;
QStringList listParName;
QStringList listParNum;

private slots:
  void closeWindow();
  void helpUse();
  void helpFlag();
  void helpParam();
  void about();
  void aboutQt();
  // socket slots
  void processLetter(miMessage&);
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
  void stationsInList();

  void updateSaveFunction( QMdiSubWindow * w );

signals:
  void statTimeReceived(const QString&);
  void newStationList(std::vector<QString>&);
  void newParameterList(const QStringList&);
  void toggleWeather();
  void toggleType();
  void saveData();
  void windowClose();
  //KTEST
  void printErrorList();

  /**
   * \brief Emitted when a new station and/or obstime has been selected in the 
   *        errorlist.
   */
  void errorListStationSelected(int station, const miutil::miTime & obstime);
};

#endif
