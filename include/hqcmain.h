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
#include "errorlist.h"
#include "datatable.h"
#include <qwidget.h>
#include <fstream>
#include <iostream>
#include <KvApp.h>
#include <qmainwindow.h>
#include <qobject.h>
#include <qpopupmenu.h> 
#include <qprocess.h> 
#include <qmenubar.h> 
#include <qmap.h>
#include <qmultilineedit.h>
#include <qpoint.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qaccel.h>
#include <qtextstream.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qpixmap.h>
#include <qobjectlist.h>
#include <qvbox.h>
#include <qtable.h>
#include <qworkspace.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qiconset.h>
#include <qcursor.h>

#include <qtClientButton.h>

#include <puTools/miString.h>
#include <miMessage.h>
#include <decodeutility/DataReinserter.h>

#include <TSPlotDialog.h>
#include "hqcdefs.h"
#include "tabwindow.h"

using namespace std;
using namespace kvalobs;
using namespace kvservice;
class DataTable;
class MDITabWindow;


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
  int nuroprpar;
  int nucoprpar;
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
  void sendSelectedParam(miutil::miString);
  /*!
   * \brief Inserts the parameters in a weather element
   *        into a listbox in the parameter dialog.
   */
  void insertParametersInListBox(int, int*);
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
  /*!
   * \brief Returns true if the hour given as input is checked in the ClockDialog
   */
  bool timeFilter(int);
  /*!
   * \brief Returns true if the given typeId and environment corresponds
   *        to a station type checked in the ListDialog 
   */
  bool hqcTypeFilter(int&, int, int);
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
  void readFromTypeIdFile(int);
  /*!
   * \brief Reads the obs_pgm table in the kvalobs database and 
   *       inserts the station information in sobsPgmList and statList
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

/*!
 * \brief Convert to "Diana-value" of range check flag 
*/
  int numCode1(int);
/*!
 * \brief Convert to "Diana-value" of consistency check flag 
*/
  int numCode2(int);
/*!
 * \brief Convert to "Diana-value" of prognostic space control flag 
*/
  int numCode3(int);
/*!
 * \brief Convert to "Diana-value" of step check flag 
*/
  int numCode4(int);
/*!
 * \brief Convert to "Diana-value" of timeseries adaption flag 
*/
  int numCode5(int);
/*!
 * \brief Convert to "Diana-value" of statistics control flag 
*/
  int numCode6(int);
/*!
 * \brief Convert to "Diana-value" of climatology control flag 
*/
  int numCode7(int);
/*!
 * \brief Convert to "Diana-value" of HQC flag 
*/
  int numCode8(int);
/*!
 * \brief Calculate the 5-digit flag-code to be shown in HQC and Diana
*/
  int getShowFlag(kvalobs::kvDataFlag);

  ListDialog* lstdlg;
  ClockDialog* clkdlg;
  DianaShowDialog* dshdlg;
  ParameterDialog* pardlg;
  TimeseriesDialog* tsdlg;
  datl tdl;
  vector<datl> datalist;
  vector<modDatl> modeldatalist;
  vector<int> stList;
  vector<int> remstList;
  listType lity;
  mettType metty;
  int selParNo[NOPARAMALL];
  currentType crT;
  vector<currentType> currentTypeList;
  DataReinserter<kvservice::KvApp> *reinserter;

public slots:
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
  int sLevel;
  int sSensor;
  QMap<int,QString> parMap;
  bool listExist;
  bool kvBaseIsUpdated;
  QString wElement;
  QString userName;
  QStringList selPar;
  int flID;
  int orID;
  int moID;
  int stID;
  int poID;
  int tyID;
  int apID;
  int taID;
  //  int otID;
  //  int huID;
  int wiID;
  int prID;
  int clID;
  int seID;
  int syID;
  int klID;
  int piID;
  int plID;
  int alID;
  bool isShFl;
  bool isShOr;
  bool isShMo;
  bool isShSt;
  bool isShPo;
  bool isShTy;
  int synopType;
  int autoobsType;
  int kvalobsType;
  bool isSynop;
  bool isAuto;
  bool isKvalobs;
  bool tsVisible;
  ClientButton* pluginB;
  QPopupMenu* file;
  int fileSaveMenuItem;
  //KTEST
  int filePrintMenuItem;
  QPopupMenu* choice;
  QPopupMenu* showmenu;
  QPopupMenu* weathermenu;
  QPopupMenu* clockmenu;
  QPopupMenu* typeIdmenu;
  QWorkspace* ws;
  QPainter* logo;
  void paintEvent(QPaintEvent*);
  // socket variables
  bool usesocket;
  bool dianaconnected;
  QString dateandTime;
  QString kvParam[NOPARAMALL];
  QString kdbParam[NOPARAMALL];
  // Database connection
  kvalobs::kvDbGate dbGate;
  dnmi::db::Connection *con;
  dnmi::db::DriverManager dbmngr;
  //  DataList dlist;
  ModelDataList mdlist;
  ObsTypeList otpList;
  std::list<kvalobs::kvObsPgm> obsPgmList;
  std::list<kvalobs::kvStation> sillist;
  std::list<kvalobs::kvStation> slist;
  std::list<kvalobs::kvParam> plist;
  std::list<long> statList;
  int order[NOPARAMALL];
  int airPressOrder[NOPARAMAIRPRESS];
  int tempOrder[NOPARAMTEMP];
  int precOrder[NOPARAMPREC];
  int visualOrder[NOPARAMVISUAL];
  int waveOrder[NOPARAMWAVE];
  int synopOrder[NOPARAMSYNOP];
  int klstatOrder[NOPARAMKLSTAT];
  int priorityOrder[NOPARAMPRIORITY];
  int windOrder[NOPARAMWIND];
  int pluOrder[NOPARAMPLU];
  
  TSPlotDialog* tspdialog; // timeseries-plot
  StationTable* stationTable;
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
  void sendImage(const miString name, const QImage& image);
  //  void sendShowText(const miString site);
  void readErrorsFromqaBase(int&, int&);


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
  // file commands
  MDITabWindow* eTable(const miutil::miTime&, 
		       const miutil::miTime&,
		       miutil::miTime&, 
		       miutil::miTime&, 
		       listType, 
		       listType, 
		       mettType, 
		       QString&,
		       int*,
		       vector<datl>&, 
		       vector<modDatl>&, 
		       list<kvStation>&,
		       int, 
		       int,
		       bool,
		       QString&);
  void closeWindow();
  void help();
  void about();
  void aboutQt();
  // socket slots
  void processLetter(miMessage&);
  void processConnect();
  void errListMenu();
  void allListMenu();
  void errLogMenu();
  void dataListMenu();
  void errLisaMenu();

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

  void updateSaveFunction( QWidget *w );

signals:
  void statTimeReceived(QString&);
  void newStationList(std::vector<QString>&);
  void newParameterList(const QStringList&);
  void toggleWeather();
  void toggleType();
  void saveData();
  //KTEST
  void printErrorList();

  /**
   * \brief Emitted when a new station and/or obstime has been selected in the 
   *        errorlist.
   */
  void errorListStationSelected(int station, const miutil::miTime & obstime);
};

#endif
