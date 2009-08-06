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
#include <q3datetimeedit.h>
#include <qlineedit.h>
#include <QTabWidget>
#include <QDialogButtonBox>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QFrame>
#include <kvcpp/KvApp.h>

using namespace kvservice;
using namespace kvalobs;
using namespace std;

namespace Weather
{
  WeatherDialog * WeatherDialog::getWeatherDialog( const kvData & data, list<kvStation>& slist, QWidget * parent, Qt::WindowFlags f )
  {
    cout << "WeatherDialog::getWeatherDialog:" << endl
	 << decodeutility::kvdataformatter::createString( data ) << endl;

    QDialog * selector = new QDialog( parent, "", Qt::WDestructiveClose );
    selector->setCaption( "Velg stasjonsinformasjon" );

    Q3VBoxLayout * mainLayout = new Q3VBoxLayout( selector );

    StationSelection * ss = new StationSelection( selector, & data );
    mainLayout->addWidget( ss );

    // Buttons:
    Q3HBoxLayout * buttonLayout = new Q3HBoxLayout( mainLayout );
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
    bool legalStation = false;
    for(list<kvalobs::kvStation>::const_iterator it=slist.begin();it!=slist.end(); it++){
      int cstnr = it->stationID();
      if ( cstnr == st ) {
	legalStation  = true;
	break;
      }
    }
    if ( !legalStation ) {
    	QMessageBox::information( ss, "WatchWeather",
				  "Ugyldig stasjonsnummer.\nVelg et annet stasjonsnummer.");
    	return 0;
    }
    miutil::miTime  cl = ss->obstime();
    int             ty = ss->typeID();
    int             se = ss->sensor() - '0';
    //    int             lv = ss->level();

    cerr << "Base for data:\n"
	 << decodeutility::kvdataformatter::createString( ss->getKvData() )
	 << endl;

    WeatherDialog * ret = 0;

    if ( st ) {
      try {
        ret= new WeatherDialog( st, cl, ty, se, 0, parent, 0, FALSE );
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

  bool WeatherDialog::sensorFilter(int gSensor, int cSensor) {
    if ( cSensor == gSensor )
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

  void WeatherDialog::setupOrigTab( SynObsList& sList, int type, QTabWidget* tabWidget ) {
    QFrame* orig = new QFrame(this);
    WeatherTable* origTab = new WeatherTable(orig, "orig", type);
    connect( tabWidget, SIGNAL(currentChanged(int) ), origTab, SLOT( showCurrentPage() ) );
    tabWidget->addTab(origTab, "Original");
  }

  void WeatherDialog::setupCorrTab( SynObsList& sList, int type, QTabWidget* tabWidget ) {
    QFrame* corr = new QFrame(this);
    WeatherTable* corrTab = new WeatherTable(corr, "corr", type);
    connect( tabWidget, SIGNAL(currentChanged(int) ), corrTab, SLOT( showCurrentPage() ) );
    tabWidget->addTab(corrTab, "Korrigert");
    cTab = corrTab;
  }

  void WeatherDialog::setupFlagTab( SynObsList& sList, int type, QTabWidget* tabWidget ) {
    QFrame* flag = new QFrame(this);
    WeatherTable* flagTab = new WeatherTable(flag, "flag", type);
    connect( tabWidget, SIGNAL(currentChanged(int) ), flagTab, SLOT( showCurrentPage() ) );
    tabWidget->addTab(flagTab, "Flagg");
  }

  void WeatherDialog::setupStationInfo() {
    if ( station == 0 ) {
      QMessageBox::information( this, "HQC - synop",
                             "Det valgte stasjonsnummer fins ikke i databasen.",
                             QMessageBox::Ok,  QMessageBox::NoButton );
      return;
    }
    QString stationDescr = QString::number( station->stationID() );
    if ( this->station )
      stationDescr += " - " + QString::fromStdString(this->station->name());
    setCaption( "Synop for stasjon " + stationDescr );
  }

  WeatherDialog::DateRange getDateRange_( std::vector<TimeObs> * timeobs )
  {
    assert( timeobs->begin() != timeobs->end() );
    miutil::miTime start = timeobs->begin()->getTime();
    miutil::miTime end = timeobs->rbegin()->getTime();
    return WeatherDialog::DateRange( start, end );
  }

  WeatherDialog::WeatherDialog( TimeObsListPtr tobs, int type, int sensor,
		      const DataReinserter<kvservice::KvApp> * dataReinserter,
		      QWidget *parent, const char* name, bool modal )
    : QDialog( parent, Qt::Window )
    , dataReinserter( dataReinserter )
    , observations( tobs )
    , station( (*StationInformation<KvApp>::getInstance( KvApp::kvApp ))[(*tobs)[0].getStation()] )
    , shownFirstTime( false )
  {
    tabWidget = new QTabWidget;
    if ( station != 0 ) {
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
      sTime.addDay(-7);
      eTime.addDay();
      WhichDataHelper whichData;
      whichData.addStation( (*station).stationID(), sTime, eTime);
      
      if ( !KvApp::kvApp->getKvData(ldList, whichData))
	cerr << "Can't connect to data table!" << endl;
      
      for(IKvObsDataList it=ldList.begin(); it!=ldList.end(); it++ ) {
	IDataList dit = it->dataList().begin();
	while( dit != it->dataList().end() ) {
	  miutil::miTime otime = (dit->obstime());
	  int typid = dit->typeID();
	  int snsor = dit->sensor();
	  if ( paramInParamsList(dit->paramID()) && typeFilter(type, dit->typeID()) && sensorFilter(sensor, dit->sensor()) ) {
	    synObs.stnr = dit->stationID();
	    synObs.otime = dit->obstime();
	    synObs.typeId[parameterIndex[dit->paramID()]] = dit->typeID();
	    synObs.sensor[parameterIndex[dit->paramID()]] = dit->sensor();
	    synObs.corr[parameterIndex[dit->paramID()]] = dit->corrected();
	    synObs.orig[parameterIndex[dit->paramID()]] = dit->original();
	    synObs.controlinfo[parameterIndex[dit->paramID()]] = dit->controlinfo().flagstring();
	    miutil::miTime protime = otime;
	    dit++;
	    otime = dit->obstime();
	    typid = dit->typeID();
	    snsor = dit->sensor();
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
	    snsor = dit->sensor();
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
    setupCorrTab(synObsList, type, tabWidget);
    setupOrigTab(synObsList, type, tabWidget);
    setupFlagTab(synObsList, type, tabWidget);

    QPushButton* saveButton = new QPushButton(tr("Lagre"));
    saveButton->setDefault(true);
    QPushButton* closeButton = new QPushButton(tr("Lukk"));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(saveButton,QDialogButtonBox::ActionRole);
    buttonBox->addButton(closeButton,QDialogButtonBox::RejectRole);
    //    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save
    //                                     | QDialogButtonBox::Close);

    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveData()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
  }


  WeatherDialog::WeatherDialog( int station, const miutil::miTime clock, int type, int sensor,
				const DataReinserter<KvApp> * dataReinserter,
				QWidget* parent, const char* name, bool modal )
    : QDialog( parent, Qt::Window)
    , dataReinserter( dataReinserter )
    , station( (*StationInformation<KvApp>::getInstance( KvApp::kvApp ))[station] )
    , shownFirstTime( false )
  {
    tabWidget = new QTabWidget;
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
    if ( !KvApp::kvApp->getKvData(ldList, whichData))
      cerr << "Can't connect to data table!" << endl;
    for(IKvObsDataList it=ldList.begin(); it!=ldList.end(); it++ ) {
      IDataList dit = it->dataList().begin();
      while( dit != it->dataList().end() ) {
	miutil::miTime otime = (dit->obstime());
        if ( paramInParamsList(dit->paramID()) && typeFilter(type, dit->typeID()) && sensorFilter(sensor, dit->sensor()-'0') ) {
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
    setupStationInfo();
    setupCorrTab(synObsList, type, tabWidget);
    setupOrigTab(synObsList, type, tabWidget);
    setupFlagTab(synObsList, type, tabWidget);
    
    QPushButton* saveButton = new QPushButton(tr("Lagre"));
    saveButton->setDefault(true);
    QPushButton* closeButton = new QPushButton(tr("Lukk"));
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(saveButton,QDialogButtonBox::ActionRole);
    buttonBox->addButton(closeButton,QDialogButtonBox::RejectRole);
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveData()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
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
                             QMessageBox::Ok,  Qt::NoButton );
      return false;
    }
    DataConsistencyVerifier::DataSet mod;
    //    cTab->getModifiedData( mod );

    //    if ( mod.empty() )
    if ( cTab->kvCorrList.empty() )
      {
	QMessageBox::information( this, "HQC - synop",
				  "Du har ingen endringer å lagre.",
				  QMessageBox::Ok );
	return true;
      }
    
    list<kvData> dl( cTab->kvCorrList.begin(), cTab->kvCorrList.end() );
    
    cerr << "Lagrer:" << endl
	 << decodeutility::kvdataformatter::createString( dl ) << endl;
    CKvalObs::CDataSource::Result_var res;
    QString changedVals;
    {
      int iii = 0;
      vector<oldNewPair>::iterator mit;
      for ( mit = cTab->oldNew.begin(); mit != cTab->oldNew.end(); mit++ ) {
	QString oldCorVal;
	QString newCorVal;
	oldCorVal = oldCorVal.setNum(mit->first,'f',1);
	newCorVal = newCorVal.setNum(mit->second,'f',1);
	QString parameterId(cTab->parm[cTab->kvCorrList[iii].paramID()]);
	changedVals += QString(cTab->kvCorrList[iii].obstime().isoTime().cStr()) + " " + parameterId + " " + oldCorVal + " --> " + newCorVal + '\n';
	iii++;
      }
      changedVals = "Følgende endringer er gjort: \n" + changedVals + "Fullføre lagring?";
      int corrMb =
      QMessageBox::question( this, "HQC - synop",
				changedVals,
                                "Ja",
				"Nei",
				"" );
      //      BusyIndicator busy;
      if ( corrMb != 1 )
	res = ri->insert( dl );
      else {
	//	emit dontStore(cTab->oldNew);
	emit dontStore();
	cTab->kvCorrList.clear();
	cTab->oldNew.clear();
	cTab->rowCol.clear();
	changedVals = "";
	return false;
      }
    }
    if ( res->res == CKvalObs::CDataSource::OK )
    {
      QMessageBox::information( this, "HQC - synop",
                                QString( "Lagret " + QString::number( dl.size() ) + " parametre til kvalobs." ),
                                QMessageBox::Ok );
      cTab->kvCorrList.clear();
      cTab->oldNew.clear();
      cTab->rowCol.clear();
      changedVals = "";
    }
    else
    {
      QMessageBox::warning( this, "HQC - synop",
                            QString( "Klarte ikke å lagre data!\n"
                                     "Feilmelding fra kvalobs var:\n") +
                            QString(res->message),
                            QMessageBox::Ok,  Qt::NoButton );
      return false;
    }
    return true;
  }

  WeatherDialog::~WeatherDialog( )
  {
  }
}

