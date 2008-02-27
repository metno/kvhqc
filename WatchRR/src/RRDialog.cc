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
#include "RRDialog.h"
#include "StationSelection.h"
#include "StationInformation.h"
#include "StationSelection.h"
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
#include <KvApp.h>
//#include <kvservice/qt/kvQtApp.h>

using namespace kvservice;
using namespace kvalobs;
using namespace std;

namespace WatchRR
{
  RRDialog * RRDialog::getRRDialog( const kvData & data, QWidget * parent )
  {
    cout << "RRDialog::getRRDialog:" << endl
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
    
    int            st = ss->station();
    miutil::miDate da = ss->obstime();
    int            ty = ss->typeID();
    int            se = ss->sensor();
    int            lv = ss->level();

    cerr << "Base for data:\n"
	 << decodeutility::kvdataformatter::createString( ss->getKvData() )
	 << endl;

    RRDialog * ret = 0;

    if ( st ) {
      try {
        ret= new RRDialog( st, da, ty, se, lv, 0, parent );
      }
      catch( invalid_argument & e ) {
      }
      catch( exception & ){
      }
    }
    return ret;
  }

  void RRDialog::setup( RRTable * rrt )
  {
//     if ( ! this->station ) {
//       ostringstream ss;
//       ss << "Ingen stasjon " << station;
//       throw invalid_argument( ss.str() );
//     }

    // Display station information:
    stationInfo = new QLineEdit( this, "Station Info" );
    stationInfo->setReadOnly( true );
    stationInfo->setFocusPolicy( NoFocus );

    QString stationDescr = QString::number( station->stationID() );
    if ( this->station )
      stationDescr += " - " + this->station->name();
    QString caption = "Nedbør for stasjon " + stationDescr;
    if ( not captionSuffix_.isEmpty() )
      caption += " [" + captionSuffix_ + "]";
    setCaption( caption );
    if ( station->environmentid() == 10 )
        stationDescr += "  (Ikke daglig)";
    stationInfo->setText( stationDescr );
    
    // Buttons:
    help   = new QPushButton( "&Hjelp", this, "Hjelp" );
    //assignToFD2 = new QCheckBox( "&Fordel manglende nedbør", this, "Assign to FD=2" );
    //assignToFD2->setChecked( true );
    save = new QPushButton( "&Lagre", this, "Lagre" );
    connect( save, SIGNAL( clicked() ), this, SLOT( saveData() ) );
//    if ( ! dataReinserter )
//      save->setEnabled( false );
    ok     = new QPushButton( "Luk&k", this, "Lukk" );
    ok->setDefault( true );
    connect( ok,   SIGNAL( clicked() ), this, SLOT( accept() ) );

    // Status Bar:
    statusBar = new QStatusBar( this, "Status Bar" );
    statusBar->setSizeGripEnabled( true );
    ttGroup = new QToolTipGroup( statusBar );
    connect( ttGroup, SIGNAL( showTip(const QString&) ),
	     statusBar, SLOT( message(const QString&) ) );
    connect( ttGroup, SIGNAL( removeTip() ),
	     statusBar, SLOT( clear() ) );

    // Table
    table = rrt;
    table->setFocus();
    //connect( assignToFD2, SIGNAL(toggled(bool)), table, SLOT(setDistributeFD2(bool)));
    
    // Layout: 
    mainLayout = new QVBoxLayout( this, 0, -1, "Main Layout" );
    {
      QHBoxLayout *topLayout = new QHBoxLayout( mainLayout, -1, "Top Layout" );
      {
	topLayout->addWidget( new QLabel("Stasjon:", this ) );
	topLayout->addWidget( stationInfo );
      }

      mainLayout->addWidget( table );

      QHBoxLayout *buttonLayout = new QHBoxLayout( mainLayout, -1, "Button Layout" );
      {
	buttonLayout->addWidget( help );
	buttonLayout->addStretch( 1 );
	//buttonLayout->addWidget( assignToFD2 );
	buttonLayout->addWidget( save );
	buttonLayout->addWidget( ok );
      }
      mainLayout->addWidget( statusBar );
    }
  }

  RRDialog::RRDialog( DayObsListPtr dol,	
		      const DataReinserter<kvservice::KvApp> * dataReinserter,
		      const QString & captionSuffix,
		      QWidget *parent, const char* name, bool modal, WFlags f )
    : QDialog( parent, name, modal, f | Qt::WDestructiveClose )
    , dataReinserter( dataReinserter )
    , captionSuffix_( captionSuffix )
    , station( (*StationInformation<KvApp>::getInstance( KvApp::kvApp ))[(*dol)[0].getStation()] )
    , shownFirstTime( false )
  {
    RRTable * rrt = new RRTable( dol, ttGroup, this, "Table" );
    setup( rrt );
    rrt->ttGroup = ttGroup;
  }


  RRDialog::RRDialog( int station, const miutil::miDate date, 
		      int type, int sensor, int level,
		      const DataReinserter<KvApp> * dataReinserter,
		      QWidget* parent, const char* name, bool modal, WFlags f )
    : QDialog( parent, name, modal, f | Qt::WDestructiveClose )
    , dataReinserter( dataReinserter )
    , station( (*StationInformation<KvApp>::getInstance( KvApp::kvApp ))[station] )
    , shownFirstTime( false )
  {
    RRTable * rrt;    
    try {
      rrt = new RRTable( station, date, type, sensor, level, 0, this, "Table" );
    }
    catch( std::runtime_error &e ) {
      int res = QMessageBox::critical( this, "HQC", 
				       QString("Feil i kontakt med kvalobs!\n"
					       "Meldingen var:\n") + e.what(),
				       QMessageBox::Retry | QMessageBox::Default,
				       QMessageBox::Abort | QMessageBox::Escape );
      if ( res == QMessageBox::Retry ) {
	try {
	  rrt = new RRTable( station, date, type, sensor, level, ttGroup, this, "Table" );
	}
	catch( std::runtime_error &e ) {
	  res = QMessageBox::critical( this, "HQC", 
				       QString("Fremdeles feil i kontakt med kvalobs!\n"
					       "Meldingen var:\n") + e.what(),
				       QMessageBox::Abort | QMessageBox::Default,
				       QMessageBox::NoButton );
	}
      }
      if ( res == QMessageBox::Abort ) {
	throw;
      }
    }
    setup ( rrt );
    rrt->ttGroup = ttGroup;
  }

  RRDialog::~RRDialog( )
  {
  }

  void RRDialog::polish()
  {
    QDialog::polish();
    table->ensureCellVisible( table->numRows() -1, 0 );	
  }

  bool RRDialog::saveData()
  {
    return table->saveData( dataReinserter );
  }

  enum ConfirmSaveValue { Yes, No, Cancel };

  ConfirmSaveValue confirmSave( RRDialog *dialog, int noOfChanges,
				const DataReinserter<KvApp> * dataReinserter ) 
  {
    ConfirmSaveValue save = (ConfirmSaveValue)
      QMessageBox::information( dialog, "HQC - Nedbør",
        QString( "Du har endret ") + QString::number( noOfChanges ) + " parametre.\n"
        "Vil du lagre endringene du har gjort?" ,
  			"&Ja", "&Nei", "&Avbryt", 0, 2 );
    if ( save == Yes ) {
      bool ok = dialog->table->saveData( dataReinserter );
      if ( ok )
        return Yes;
      else
        return Cancel;
    }
    return save;
  }

  void RRDialog::showEvent( QShowEvent * e )
  {
    QDialog::showEvent( e );
    if ( not shownFirstTime ) {
      shownFirstTime = true;
      table->ensureCellVisible( table->numRows() -1, 0 );
    }
  }


  void RRDialog::closeEvent( QCloseEvent * e )
  {
    QDialog::closeEvent( e );
  }

  void RRDialog::reject()
  {
    // If user presses escape, reject will be called, but not closeEvent
    accept();
  }

  void RRDialog::accept()
  {
    DataConsistencyVerifier::DataSet mod;
    table->getModifiedData( mod );
    
    if ( mod.empty() ) {
      QDialog::accept();
      return;
    }
    
    ConfirmSaveValue saved = confirmSave( this, mod.size(), dataReinserter );
    if ( saved == Yes )
      QDialog::accept();
    else if ( saved == No )
      QDialog::reject();
  }
}
