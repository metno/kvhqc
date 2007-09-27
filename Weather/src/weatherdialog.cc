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
#include "weatherdialog.h"
#include "StationSelection.h"
#include "StationInformation.h"
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qstatusbar.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qdialog.h>
#include <qdatetimeedit.h>
#include <qlineedit.h>
#include <qvbox.h>
#include <qtabdialog.h>
#include <qtable.h>
#include <KvApp.h>

using namespace kvservice;
using namespace kvalobs;
using namespace std;

namespace Weather
{
  WeatherDialog * WeatherDialog::getWeatherDialog( const kvData & data, QWidget * parent )
  {
    cout << "WeatherDialog::getWeatherDialog:" << endl
	 << decodeutility::kvdataformatter::createString( data ) << endl;

    QDialog * selector = new QDialog( parent, "", WDestructiveClose );
    selector->setCaption( "Velg stasjonsinformasjon" );

    QVBoxLayout * mainLayout = new QVBoxLayout( selector );

    StationSelection * ss = new StationSelection( selector, & data );
    mainLayout->addWidget( ss );

    // Buttons:
    QHBoxLayout * buttonLayout = new QHBoxLayout( mainLayout );
    buttonLayout->addStretch( 1 );
    
    QPushButton * ok = new QPushButton( "&Ok", selector );
    buttonLayout->addWidget( ok );
    connect( ok, SIGNAL( clicked() ), selector, SLOT( accept() ) );

    QPushButton * can = new QPushButton( "&Avbryt", selector );
    buttonLayout->addWidget( can );
    connect( can, SIGNAL( clicked() ), selector, SLOT( reject() ) );

    const int result = selector->exec();
    if ( result == QDialog::Rejected )
      return 0;
    
    int             st = ss->station();
    miutil::miTime  cl = ss->obstime();
    int             ty = ss->typeID();
    //    int             se = ss->sensor();
    //    int             lv = ss->level();

    cerr << "Base for data:\n"
	 << decodeutility::kvdataformatter::createString( ss->getKvData() )
	 << endl;

    WeatherDialog * ret = 0;

    if ( st ) {
      try {
        ret= new WeatherDialog( st, cl, ty, 0, parent );
      }
      catch( invalid_argument & e ) {
      }
    }
    return ret;
  }

  bool WeatherDialog::typeFilter(int gType, int cType) {
    if ( gType == 0 ) 
      return true;
    else if ( abs(cType) == gType )
      return true;
    else
      return false;
  }
  
  bool WeatherDialog::paramInParamsList(int stnr) {
    bool pipl = false;
    for ( int i = 0; i < NP; i++ ) {
      if ( stnr == params[i] )
	pipl = true;
    }
    return pipl;
  }

  void WeatherDialog::setupOrigTab( SynObsList& sList, int type ) {
    QVBox* orig = new QVBox(this, "orig");    
    orig->setMargin(5);
    WeatherTable* origTab = new WeatherTable(orig, type);
    origTab->resize( origTab->sizeHint() );
    addTab(orig, "Original");
  }

  void WeatherDialog::setupCorrTab( SynObsList& sList, int type ) {
    //    mainLayout = new QVBoxLayout( this, 0, -1, "Main Layout" );
    QVBox* corr = new QVBox(this, "corr");    
    corr->setMargin(5);
    //    ttGroup = new QToolTipGroup( this );
    //    WeatherTable* corrTab = new WeatherTable( numRows, numCols, ttGroup, corr);
    WeatherTable* corrTab = new WeatherTable(corr, type);
    corrTab->resize( corrTab->sizeHint() );
    addTab(corr, "Korrigert");
    cTab = corrTab;
    //    mainLayout->addWidget( cTab );
    //    mainLayout->addWidget( statusBar );
  }

  void WeatherDialog::setupFlagTab( SynObsList& sList, int type ) {
    QVBox* flag = new QVBox(this, "flag");    
    flag->setMargin(5);
    //    statusBar = new QStatusBar( this, "Status Bar" );
    //    statusBar->setSizeGripEnabled( true );
    //    ttGroup = new QToolTipGroup( statusBar );
    WeatherTable* flagTab = new WeatherTable(flag, type);
    flagTab->resize( flagTab->sizeHint() );
    addTab(flag, "Flagg");
  }

  void WeatherDialog::setupStationInfo() {
    if ( station == 0 ) {
      QMessageBox::information( this, "HQC - synop",
                             "Det valgte stasjonsnummer fins ikke i databasen.",
                             QMessageBox::Ok,  QMessageBox::NoButton );
      setApplyButton("Lagre");
      setCancelButton("Lukk");
      emit cancelButtonPressed();
      return;
    }
    QString stationDescr = QString::number( station->stationID() );
    if ( this->station )
      stationDescr += " - " + this->station->name();
    setCaption( "Synop for stasjon " + stationDescr );
  }

  WeatherDialog::DateRange getDateRange_( std::vector<TimeObs> * timeobs )
  {
    assert( timeobs->begin() != timeobs->end() );
    miutil::miTime start = timeobs->begin()->getTime();
    miutil::miTime end = timeobs->rbegin()->getTime();
    return WeatherDialog::DateRange( start, end );
  }

  WeatherDialog::WeatherDialog( TimeObsListPtr tobs, int type,	
		      const DataReinserter<kvservice::KvApp> * dataReinserter,
		      QWidget *parent, const char* name, bool modal, WFlags f )
    : QTabDialog( parent, name, modal )
    , dataReinserter( dataReinserter )
    , observations( tobs )
    , station( (*StationInformation<KvApp>::getInstance( KvApp::kvApp ))[(*tobs)[0].getStation()] )
    , shownFirstTime( false )
  {  
    //    mainLayout = new QVBoxLayout( this, 0, -1, "Main Layout" );
    //    statusBar = new QStatusBar( this, "Status Bar" );
    //    statusBar->setSizeGripEnabled( true );
    /*    if ( station == 0 ) {
      QMessageBox::information( this, "HQC - synop",
                             "Det valgte stasjonsnummer fins ikke i databasen.",
                             QMessageBox::Ok,  QMessageBox::NoButton );
      setApplyButton("Lagre");
      setCancelButton("Lukk");
      emit cancelButtonPressed();
      return;
    }
    */
    if ( station != 0 ) {
    connect( this, SIGNAL( applyButtonPressed() ), this, SLOT( saveData() ) );
    //    statusBar = new QStatusBar( this, "Status Bar" );
    //    statusBar->setSizeGripEnabled( true );
    //    ttGroup = new QToolTipGroup( statusBar );
    for ( int i = 0; i < NP; i++ ) {
      parameterIndex[params[i]] = i;
    }
    synObsList.clear();
    for ( int ip = 0; ip < NP; ip++) {
      synObs.orig[ip]   = -32767.0;
      synObs.corr[ip]   = -32767.0;
      synObs.controlinfo[ip] = "";
    }
    miutil::miTime sTime = (*tobs)[0].getTime();
    miutil::miTime eTime = (*tobs)[0].getTime();
    //    int type = (*tobs)[0].getType();
    sTime.addDay(-7);
    eTime.addDay();
    WhichDataHelper whichData;
    whichData.addStation( (*station).stationID(), sTime, eTime);

    KvObsDataList ldList;
    if ( !KvApp::kvApp->getKvData(ldList, whichData))
      cerr << "Can't connect to data table!" << endl;


    /*
    for(IKvObsDataList it=ldList.begin(); it!=ldList.end(); it++ ) {
      IDataList dit = it->dataList().begin();
      while( dit != it->dataList().end() ) {
	dit++;
      }
    }
    */


    for(IKvObsDataList it=ldList.begin(); it!=ldList.end(); it++ ) {
      IDataList dit = it->dataList().begin();
      while( dit != it->dataList().end() ) {
	miutil::miTime otime = (dit->obstime());
	int typid = dit->typeID();
        if ( paramInParamsList(dit->paramID()) && typeFilter(type, dit->typeID()) ) {
	  synObs.stnr = dit->stationID();
	  synObs.otime = dit->obstime();
	  synObs.typeId[parameterIndex[dit->paramID()]] = dit->typeID();
	  synObs.corr[parameterIndex[dit->paramID()]] = dit->corrected();
	  synObs.orig[parameterIndex[dit->paramID()]] = dit->original();
	  synObs.controlinfo[parameterIndex[dit->paramID()]] = dit->controlinfo().flagstring();
	  miutil::miTime protime = otime;
	  dit++;
	  otime = dit->obstime();
	  typid = dit->typeID();
	  if ( otime != protime ) {
	    synObsList.push_back(synObs);
	    for ( int ip = 0; ip < NP; ip++) {
	      synObs.orig[ip]   = -32767.0;
	      synObs.corr[ip]   = -32767.0;
	      synObs.controlinfo[ip] = "";
	    }
	  }
	}
	else {
	  miutil::miTime protime = otime;
	  dit++;
	  otime = dit->obstime();
	  typid = dit->typeID();
	  if ( otime != protime ) {
       	    synObsList.push_back(synObs);
	    for ( int ip = 0; ip < NP; ip++) {
	      synObs.orig[ip]   = -32767.0;
	      synObs.corr[ip]   = -32767.0;
	      synObs.controlinfo[ip] = "";
	    }
	  }
	}
      }
    }
    }
    setupStationInfo();
    setupCorrTab(synObsList, type);
    setupOrigTab(synObsList, type);
    setupFlagTab(synObsList, type);
    
    //    setupStationInfo();
    QString OK;
    setOkButton(OK);
    setApplyButton("Lagre");
    setCancelButton("Lukk");
  }


  WeatherDialog::WeatherDialog( int station, const miutil::miTime clock, int type,
				const DataReinserter<KvApp> * dataReinserter,
				QWidget* parent, const char* name, bool modal, WFlags f )
  //    : QDialog( parent, name, modal, f | Qt::WDestructiveClose )
    : QTabDialog( parent, name, modal)
    , dataReinserter( dataReinserter )
    , station( (*StationInformation<KvApp>::getInstance( KvApp::kvApp ))[station] )
    , shownFirstTime( false )
  {
    /*    observations = getTimeObs( station, clock, clock );
    list<kvalobs::kvData *> okdl;
    TimeObs & dobs = (*observations)[0];
    dobs.getAll( okdl );
    */
    connect( this, SIGNAL( applyButtonPressed() ), this, SLOT( saveData() ) );
    for ( int i = 0; i < NP; i++ )
      parameterIndex[params[i]] = i;
    synObsList.clear();
    for ( int ip = 0; ip < NP; ip++) {
      synObs.orig[ip]   = -32767.0;
      synObs.corr[ip]   = -32767.0;
      synObs.controlinfo[ip] = "";
    }
    miutil::miTime sTime = clock;
    miutil::miTime eTime = clock;
    sTime.addDay(-7);
    eTime.addDay();
    WhichDataHelper whichData;
    whichData.addStation(station, sTime, eTime);
    KvObsDataList ldList;
    if ( !KvApp::kvApp->getKvData(ldList, whichData))
      cerr << "Can't connect to data table!" << endl;
    for(IKvObsDataList it=ldList.begin(); it!=ldList.end(); it++ ) {
      IDataList dit = it->dataList().begin();
      while( dit != it->dataList().end() ) {
	miutil::miTime otime = (dit->obstime());
        if ( paramInParamsList(dit->paramID()) && typeFilter(type, dit->typeID()) ) {
	  synObs.stnr = dit->stationID();
	  synObs.otime = dit->obstime();
	  synObs.typeId[parameterIndex[dit->paramID()]] = dit->typeID();
	  synObs.corr[parameterIndex[dit->paramID()]] = dit->corrected();
	  synObs.orig[parameterIndex[dit->paramID()]] = dit->original();
	  synObs.controlinfo[parameterIndex[dit->paramID()]] = dit->controlinfo().flagstring();
	  miutil::miTime protime = otime;
	  dit++;
	  otime = dit->obstime();
	  if ( otime != protime ) {
	    synObsList.push_back(synObs);
	    for ( int ip = 0; ip < NP; ip++) {
	      synObs.orig[ip]   = -32767.0;
	      synObs.corr[ip]   = -32767.0;
	      synObs.controlinfo[ip] = "";
	    }
	  }
	}
	else {
	  miutil::miTime protime = otime;
	  dit++;
	  otime = dit->obstime();
	  if ( otime != protime ) {
	    synObsList.push_back(synObs);
	    for ( int ip = 0; ip < NP; ip++) {
	      synObs.orig[ip]   = -32767.0;
	      synObs.corr[ip]   = -32767.0;
	      synObs.controlinfo[ip] = "";
	    }
	  }
	}
      }
    }
    /*    
    miutil::miTime protime;
    for(IKvObsDataList it=ldList.begin(); it!=ldList.end(); it++ ) {
      IDataList dit = it->dataList().begin();
      while( dit != it->dataList().end() ) {
	miutil::miTime otime = (dit->obstime());
	int prType;
	int prParam;
        if ( paramInParamsList(dit->paramID()) && type == dit->typeID() ) {
	  synObs.stnr = dit->stationID();
	  synObs.otime = dit->obstime();
	  synObs.corr[parameterIndex[dit->paramID()]] = dit->corrected();
	  synObs.orig[parameterIndex[dit->paramID()]] = dit->original();
	  synObs.controlinfo[parameterIndex[dit->paramID()]] = dit->controlinfo().flagstring();
	}
	protime = otime;
	prType = dit->typeID();
	prParam = dit->paramID();
	dit++;
	otime = dit->obstime();
	if ( otime != protime ) {
	  synObsList.push_back(synObs);
	  for ( int ip = 0; ip < NP; ip++) {
	    synObs.orig[ip]   = -32767.0;
	    synObs.corr[ip]   = -32767.0;
	    synObs.controlinfo[ip] = "";
	  }
	}
	
	else {
	  protime = otime;
	  prParam = dit->paramID();
	  prType = dit->typeID();
       	  dit++;
	}
	
      }
    }
    */
    setupStationInfo();
    setupCorrTab(synObsList, type);
    setupOrigTab(synObsList, type);
    setupFlagTab(synObsList, type);
    QString OK;
    setOkButton(OK);
    setApplyButton("Lagre");
    setCancelButton("Lukk");
  }

  bool WeatherDialog::saveData()
  {
    saveData( dataReinserter );
    return true;
  }

  bool WeatherDialog::saveData(const DataReinserter<kvservice::KvApp> *ri)
  {
    if ( ! ri )
    {
      QMessageBox::critical( this, "HQC - synop",
                             "Du er ikke autorisert til å lagre data i kvalobs.",
                             QMessageBox::Ok,  QMessageBox::NoButton );
      return false;
    }
    DataConsistencyVerifier::DataSet mod;
    cTab->getModifiedData( mod );

    if ( mod.empty() )
    {
      QMessageBox::information( this, "HQC - synop",
                                "Du har ingen endringer å lagre.",
                                QMessageBox::Ok );
      return true;
    }

    list<kvData> dl( mod.begin(), mod.end() );

    cerr << "Lagrer:" << endl
    << decodeutility::kvdataformatter::createString( dl ) << endl;
    CKvalObs::CDataSource::Result_var res;
    {
      //      BusyIndicator busy;
      res = ri->insert( dl );
    }
    if ( res->res == CKvalObs::CDataSource::OK )
    {
      QMessageBox::information( this, "HQC - synop",
                                QString( "Lagret " + QString::number( dl.size() ) + " parametre til kvalobs." ),
                                QMessageBox::Ok );
    }
    else
    {
      QMessageBox::warning( this, "HQC - synop",
                            QString( "Klarte ikke å lagre data!\n"
                                     "Feilmelding fra kvalobs var:\n") +
                            res->message,
                            QMessageBox::Ok,  QMessageBox::NoButton );
      return false;
    }

    for ( list<kvData>::const_iterator it = dl.begin(); it != dl.end(); ++ it )
    {
      miutil::miTime t = it->obstime();
      miutil::miDate d = t.date();
      //      if ( t.hour() > 7 )
      //        d.addDay();
      for ( std::vector<TimeObs>::iterator tobs = observations->begin(); tobs != observations->end(); ++ tobs )
      {
	//      if ( tobs->getDate() == d )
	//      {
          kvData & data = tobs->get( it->paramID(), it->obstime() );
          data = * it;
	  //      }
      }
    }
    return true;
  }
  WeatherDialog::~WeatherDialog( )
  {
  }
}

