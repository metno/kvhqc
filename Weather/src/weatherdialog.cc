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

#include "mi_foreach.hh"
#include "StationSelection.h"
#include "StationInformation.h"

#include <kvcpp/KvApp.h>

#include <QtGui/qlabel.h>
#include <QtGui/qpushbutton.h>
#include <QtGui/qcheckbox.h>
#include <QtGui/qstatusbar.h>
#include <QtGui/qlayout.h>
#include <QtGui/qapplication.h>
#include <QtGui/qmessagebox.h>
#include <QtGui/qdialog.h>
#include <QtGui/qlineedit.h>
#include <QtGui/QTabWidget>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <Qt3Support/q3datetimeedit.h>
#include <Qt3Support/Q3HBoxLayout>
#include <Qt3Support/Q3VBoxLayout>

using namespace kvservice;
using namespace kvalobs;
using namespace std;

namespace Weather
{
  WeatherDialog * WeatherDialog::getWeatherDialog( const kvData & data, list<kvStation>& slist, QWidget * parent, Qt::WindowFlags f )
  {
    cout << "WeatherDialog::getWeatherDialog:" << endl
	 << decodeutility::kvdataformatter::createString( data ) << endl;

    QDialog * selector = new QDialog( parent, "", Qt::WDestructiveClose|f );
    selector->setCaption( tr("Velg stasjonsinformasjon") );

    Q3VBoxLayout * mainLayout = new Q3VBoxLayout( selector );

    StationSelection * ss = new StationSelection( selector, & data );
    mainLayout->addWidget( ss );

    // Buttons:
    Q3HBoxLayout * buttonLayout = new Q3HBoxLayout( mainLayout );
    buttonLayout->addStretch( 1 );

    QPushButton * ok = new QPushButton( tr("&Ok"), selector );
    buttonLayout->addWidget( ok );
    connect( ok, SIGNAL( clicked() ), selector, SLOT( accept() ) );

    QPushButton * can = new QPushButton( tr("&Avbryt"), selector );
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
				  tr("Ugyldig stasjonsnummer.\nVelg et annet stasjonsnummer."));
    	return 0;
    }
    timeutil::ptime cl = ss->obstime();
    int             ty = ss->typeID();
    int             se = ss->sensor() - '0';
    //    int             lv = ss->level();

    cerr << "Base for data:\n"
	 << decodeutility::kvdataformatter::createString( ss->getKvData() )
	 << endl;

    WeatherDialog * ret = 0;

    if ( st ) {
      try {
        ret = new WeatherDialog(st, cl, ty, se, 0, parent);
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
    if( cSensor >= '0' and cSensor <= '9' )
        cSensor -= '0';
    return (cSensor == gSensor);
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
    tabWidget->addTab(origTab, tr("Original"));
  }

  void WeatherDialog::setupCorrTab( SynObsList& sList, int type, QTabWidget* tabWidget ) {
    QFrame* corr = new QFrame(this);
    WeatherTable* corrTab = new WeatherTable(corr, "corr", type);
    connect( tabWidget, SIGNAL(currentChanged(int) ), corrTab, SLOT( showCurrentPage() ) );
    tabWidget->addTab(corrTab, tr("Korrigert"));
    cTab = corrTab;
  }

  void WeatherDialog::setupFlagTab( SynObsList& sList, int type, QTabWidget* tabWidget ) {
    QFrame* flag = new QFrame(this);
    WeatherTable* flagTab = new WeatherTable(flag, "flag", type);
    connect( tabWidget, SIGNAL(currentChanged(int) ), flagTab, SLOT( showCurrentPage() ) );
    tabWidget->addTab(flagTab, tr("Flagg"));
  }

  void WeatherDialog::setupStationInfo() {
    if ( station == 0 ) {
      QMessageBox::information( this, tr("HQC - synop"),
                             tr("Det valgte stasjonsnummer fins ikke i databasen."),
                             QMessageBox::Ok,  QMessageBox::NoButton );
      return;
    }
    QString stationDescr = QString::number( station->stationID() );
    if ( this->station )
      stationDescr += " - " + QString::fromStdString(this->station->name());
    setCaption( tr("Synop for stasjon %1").arg(stationDescr) );
  }

  WeatherDialog::DateRange getDateRange_( std::vector<TimeObs> * timeobs )
  {
    assert( timeobs->begin() != timeobs->end() );
    timeutil::ptime start = timeobs->begin()->getTime();
    timeutil::ptime end = timeobs->rbegin()->getTime();
    return WeatherDialog::DateRange( start, end );
  }

WeatherDialog::WeatherDialog(TimeObsListPtr tobs, int type, int sensor,
                             const DataReinserter<kvservice::KvApp> * dataReinserter,
                             QWidget *parent)
    : QDialog( parent, Qt::Window )
    , dataReinserter( dataReinserter )
    , station( (*StationInformation<KvApp>::getInstance( KvApp::kvApp ))[(*tobs)[0].getStation()] )
    , shownFirstTime( false )
{
    initData((*tobs)[0].getTime(), type, sensor);
    setupGUI(type);
}

WeatherDialog::WeatherDialog(int station, const timeutil::ptime& clock, int type, int sensor,
                             const DataReinserter<KvApp> * dataReinserter,
                             QWidget* parent)
    : QDialog( parent, Qt::Window)
    , dataReinserter( dataReinserter )
    , station( (*StationInformation<KvApp>::getInstance( KvApp::kvApp ))[station] )
    , shownFirstTime( false )
{
    initData(clock, type, sensor);
    setupGUI(type);
}

void WeatherDialog::initData(const timeutil::ptime& clock, int type, int sensor)
{
    if( station == 0 )
        return;
    const int stationID = station->stationID();

    OpgmList opgtl;
    if( !KvApp::kvApp->getKvObsPgm(obsPgmList, statList, false) ) {
        cerr << "Ingen obspgm" << endl;
    } else {
        mi_foreach(const kvalobs::kvObsPgm& op, obsPgmList) {
            if( op.stationID() == stationID ) {
                cerr << op.stationID() << " "
                     << setw(3) << op.paramID() << " "
                     << setw(3) << op.typeID() <<  " "
                     << op.kl00() << " "
                     << op.kl01() << " "
                     << op.kl02() << " "
                     << op.kl03() << " "
                     << op.kl04() << " "
                     << op.kl05() << " "
                     << op.kl06() << " "
                     << op.kl07() << " "
                     << op.kl08() << " "
                     << op.kl09() << " "
                     << op.kl10() << " "
                     << op.kl11() << " "
                     << op.kl12() << " "
                     << op.kl13() << " "
                     << op.kl14() << " "
                     << op.kl15() << " "
                     << op.kl16() << " "
                     << op.kl17() << " "
                     << op.kl18() << " "
                     << op.kl19() << " "
                     << op.kl20() << " "
                     << op.kl21() << " "
                     << op.kl22() << " "
                     << op.kl23() << " "
                     << op.fromtime() << " "
                     << op.totime() << endl;
                opgmList(opgtl, op);
            }
        }
    }

    synObsList.clear();
    for( int i = 0; i < NP; i++ ) {
        parameterIndex[params[i]] = i;
        synObs.orig[i] = -32767.0;
        synObs.corr[i] = -32767.0;
        synObs.controlinfo[i] = "";
    }
    timeutil::ptime sTime = clock, eTime = clock;
    sTime += boost::gregorian::days(-7);
    eTime += boost::gregorian::days(1);
    WhichDataHelper whichData;
    whichData.addStation( stationID, timeutil::to_miTime(sTime), timeutil::to_miTime(eTime));
    
    if ( !KvApp::kvApp->getKvData(ldList, whichData))
        cerr << "Can't connect to data table!" << endl;
    if( ldList.empty() or ldList.front().dataList().empty() ) {
        std::cerr << "Empty data list." << std::endl;
        return;
    }
    int ohour = sTime.time_of_day().hours(), nexthour = ohour;
    timeutil::ptime protime = timeutil::from_miTime(ldList.front().dataList().front().obstime());
    for(IKvObsDataList it = ldList.begin(); it!=ldList.end(); it++ ) {
        if( it->dataList().empty() )
            continue;
        IDataList dit = it->dataList().begin();
        timeutil::ptime otime = timeutil::from_miTime(dit->obstime()), nexttime = otime;
        int nuOtimes = 0;
        mi_foreach(const int opg_hour, opgtl) {
            if( opg_hour == otime.time_of_day().hours() ) {
                ohour = opg_hour;
                break;
            }
            nuOtimes++;
        }
        if ( nuOtimes == (int)opgtl.size() )
            cerr << "Obstime not in obs_pgm " << dit->obstime() << endl;
        
        nexttime = protime;
        int hd = nexthour-nexttime.time_of_day().hours();
        if ( hd < 0 )
            hd += 24;
        nexttime += boost::posix_time::hours(hd);
        
        while ( otime > nexttime ) {
            
            int hourDiff = otime.time_of_day().hours() - nexthour;
            
            if ( hourDiff < 0 )
                hourDiff += 24;
            
            synObs.stnr = dit->stationID();
            synObs.otime = nexttime;
            //	synObs.otime.addHour(-hourDiff);
            for ( int ip = 0; ip < NP; ip++) {
                synObs.orig[ip]   = -32767.0;
                synObs.corr[ip]   = -32767.0;
                synObs.controlinfo[ip] = "";
            }
            synObsList.push_back(synObs);
            ++nexthour;
            nexttime += boost::posix_time::hours(1);
        }
        
        while( dit != it->dataList().end() ) {
            otime = timeutil::from_miTime(dit->obstime());
            
            if ( paramInParamsList(dit->paramID())
                 && typeFilter(type, dit->typeID())
                 && sensorFilter(sensor, dit->sensor()) ) {
                synObs.stnr = dit->stationID();
                synObs.otime = timeutil::from_miTime(dit->obstime());
                synObs.typeId[parameterIndex[dit->paramID()]] = dit->typeID();
                synObs.sensor[parameterIndex[dit->paramID()]] = dit->sensor();
                synObs.corr[parameterIndex[dit->paramID()]] = dit->corrected();
                synObs.orig[parameterIndex[dit->paramID()]] = dit->original();
                synObs.controlinfo[parameterIndex[dit->paramID()]] = dit->controlinfo().flagstring();
                nexthour = ohour;
                protime = otime;
                dit++;
                // FIXME this is a hack
                if( dit == it->dataList().end() )
                    protime += boost::posix_time::hours(-1);
                else
                    otime = timeutil::from_miTime(dit->obstime());
                // UNUSED typid = dit->typeID();
                // UNUSED snsor = dit->sensor();
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
                protime = otime;
                dit++;
                // FIXME this is a hack
                if( dit == it->dataList().end() )
                    protime += boost::posix_time::hours(-1);
                else
                    otime = timeutil::from_miTime(dit->obstime());
                // UNUSED typid = dit->typeID();
                // UNUSED snsor = dit->sensor();
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

void WeatherDialog::setupGUI(int type)
{
    tabWidget = new QTabWidget;
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

  bool WeatherDialog::saveData()
  {
    saveData( dataReinserter );
    return true;
  }

  bool WeatherDialog::saveData(const DataReinserter<kvservice::KvApp> *ri)
  {
    if ( ! ri )
    {
      QMessageBox::critical( this, tr("HQC - synop"),
                             tr("Du er ikke autorisert til å lagre data i kvalobs."),
                             QMessageBox::Ok,  Qt::NoButton );
      return false;
    }
    DataConsistencyVerifier::DataSet mod;
    //    cTab->getModifiedData( mod );

    //    if ( mod.empty() )
    if ( cTab->kvCorrList.empty() )
      {
	QMessageBox::information( this, tr("HQC - synop"),
				  tr("Du har ingen endringer å lagre."),
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
	changedVals += QString::fromStdString(timeutil::to_iso_extended_string(timeutil::from_miTime(cTab->kvCorrList[iii].obstime())))
            + " " + parameterId + " " + oldCorVal + " --> " + newCorVal + '\n';
	iii++;
      }
      changedVals = tr("Følgende endringer er gjort: \n") + changedVals + tr("Fullføre lagring?");
      int corrMb =
      QMessageBox::question( this, tr("HQC - synop"),
				changedVals,
                                tr("Ja"),
				tr("Nei"),
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
      QMessageBox::information( this, tr("HQC - synop"),
                                tr( "Lagret %1 parametre til kvalobs.").arg(dl.size()),
                                QMessageBox::Ok );
      cTab->kvCorrList.clear();
      cTab->oldNew.clear();
      cTab->rowCol.clear();
      changedVals = "";
    }
    else
    {
      QMessageBox::warning( this, tr("HQC - synop"),
                            tr("Klarte ikke å lagre data!\n"
                               "Feilmelding fra kvalobs var:\n%1").arg(QString(res->message)),
                            QMessageBox::Ok,  Qt::NoButton );
      return false;
    }
    return true;
  }
  
void WeatherDialog::opgmList( OpgmList& opgtl, const kvalobs::kvObsPgm& op)
{
    if ( op.kl00() ) opgtl.push_back( 0);
    if ( op.kl01() ) opgtl.push_back( 1);
    if ( op.kl02() ) opgtl.push_back( 2);
    if ( op.kl03() ) opgtl.push_back( 3);
    if ( op.kl04() ) opgtl.push_back( 4);
    if ( op.kl05() ) opgtl.push_back( 5);
    if ( op.kl06() ) opgtl.push_back( 6);
    if ( op.kl07() ) opgtl.push_back( 7);
    if ( op.kl08() ) opgtl.push_back( 8);
    if ( op.kl09() ) opgtl.push_back( 9);
    if ( op.kl10() ) opgtl.push_back(10);
    if ( op.kl11() ) opgtl.push_back(11);
    if ( op.kl12() ) opgtl.push_back(12);
    if ( op.kl13() ) opgtl.push_back(13);
    if ( op.kl14() ) opgtl.push_back(14);
    if ( op.kl15() ) opgtl.push_back(15);
    if ( op.kl16() ) opgtl.push_back(16);
    if ( op.kl17() ) opgtl.push_back(17);
    if ( op.kl18() ) opgtl.push_back(18);
    if ( op.kl19() ) opgtl.push_back(19);
    if ( op.kl20() ) opgtl.push_back(20);
    if ( op.kl21() ) opgtl.push_back(21);
    if ( op.kl22() ) opgtl.push_back(22);
    if ( op.kl23() ) opgtl.push_back(23);
    opgtl.sort();
    opgtl.unique();
}

  WeatherDialog::~WeatherDialog( )
  {
  }
}
