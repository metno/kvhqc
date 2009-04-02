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
#include "MultiStationSelection.h"
#include "StationSelection.h"
#include "weatherdialog.h"
#include "BusyIndicator.h"
#include <puTools/miTime>
#include <kvcpp/KvApp.h>
#include <q3listview.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QKeyEvent>
#include <Q3VBoxLayout>
#include <boost/thread.hpp>
#include <q3tabdialog.h>

#include <cassert>

#include <iostream>

using namespace kvalobs;
using namespace miutil;
using namespace std;

namespace Weather
{
  namespace
  {
    class MSSListView : public Q3ListView
    {
    public:
      MSSListView( QWidget * parent )
    	: Q3ListView( parent )
      {
      }
    protected:
      virtual void keyPressEvent( QKeyEvent * e )
      {
    	if ( e->key() == Qt::Key_Delete )
    	  delete currentItem();
    	else
    	  Q3ListView::keyPressEvent( e );
      }
    };
    struct MSSListItem
      : public Q3ListViewItem
    {
      const kvalobs::kvData data;

      MSSListItem( Q3ListView * parent, const kvalobs::kvData & data )
    	: Q3ListViewItem( parent ), data( data )
      {
    	setText( 0, QString::number( data.stationID() ) );
    	setText( 1, QString( data.obstime().isoDate().cStr() ) );
    	setText( 2, QString::number( data.typeID() ) );
	setText( 3, QString::number( data.sensor() - '0') );
	//    	setText( 4, QString::number( data.level() ) );
      }
    };
  }

  MultiStationSelection::MultiStationSelection(std::list<kvStation>& slist, QWidget * parent, const kvData * data )
    : QDialog( parent )
    , slist_(slist)
  {
    Q3VBoxLayout * mainLayout = new Q3VBoxLayout( this );
    {
      Q3HBoxLayout * topLayout = new Q3HBoxLayout( mainLayout );
      {
    	selector = new StationSelection( this, data );
    	topLayout->addWidget( selector );
    	Q3VBoxLayout * buttons = new Q3VBoxLayout( topLayout );
    	{
    	  buttons->addStretch( 1 );
    	  QPushButton * transfer = new QPushButton( "&V", this );
    	  connect( transfer, SIGNAL( clicked() ),
    		       this, SLOT( doTransfer() ) );
    	  buttons->addWidget( transfer );
    	}
      }

      stations = new MSSListView( this );
      stations->setSelectionMode( Q3ListView::Single );
      stations->addColumn( "Stasjon" );
      stations->addColumn( "Tid" );
      stations->addColumn( "Type" );
      stations->addColumn( "Sensor" );
      //      stations->addColumn( "Lvl" );
      mainLayout->addWidget( stations );

      Q3HBoxLayout * buttonLayout = new Q3HBoxLayout( mainLayout );
      {
    	buttonLayout->addStretch( 1 );
    	QPushButton * ok = new QPushButton( "&Ok", this );
    	buttonLayout->addWidget( ok );
    	connect( ok, SIGNAL( clicked() ), this, SLOT( start() ) );
    	QPushButton * can = new QPushButton( "&Lukk", this );
    	buttonLayout->addWidget( can );
    	connect( can, SIGNAL( clicked() ), this, SLOT( reject() ) );
      }
    }
  }

  MultiStationSelection::~MultiStationSelection( )
  {
  }

  void MultiStationSelection::doTransfer()
  {
    int cstnr = selector->getKvData().stationID();
    bool legalStation = false;
    for(std::list<kvalobs::kvStation>::const_iterator sit=slist_.begin();sit!=slist_.end(); sit++){
      int stnr = sit->stationID();
      if ( cstnr == stnr ) {
	legalStation  = true;
	break;
      }
    }
    if ( !legalStation ) {
      cerr << "Ugyldig stasjonsnummer er " << cstnr << endl;
      cerr << "legalStation er " << legalStation << endl;
      QMessageBox::information( this, "WatchRR",
				"Ugyldig stasjonsnummer.\nVelg et annet stasjonsnummer.");
      return;
    }
    MSSListItem * item = new MSSListItem( stations, selector->getKvData() );
    Q3ListViewItem * it = dynamic_cast<Q3ListViewItem *>( item );
    assert( it );
    stations->insertItem( it );
  }

  pair<miTime, miTime> dates_( const miTime & d )
  {
    miTime start( d );
    start.addDay(-4);
    miTime stop( d );
    stop.addDay();
    pair<miTime, miTime> dates( start, stop );
    return dates;
  }


  void MultiStationSelection::start()
  {
    Q3ListViewItem * it = stations->firstChild();
    if ( ! it )
      return;
    MSSListItem * ss = dynamic_cast<MSSListItem *>( it );
    assert( ss );
    const kvData * d = & ss->data;
    int type = d->typeID();
    int sensor = d->sensor();

      pair<miTime, miTime> dates = dates_( d->obstime() );
    TimeObsListPtr next;
    try {
      BusyIndicator busy;
      //      next = getTimeObs( d->stationID(), dates.first, dates.second );
      next = getTimeObs( d->stationID(), d->obstime(), d->obstime(), d->typeID() );
    }
    catch( std::runtime_error & ) {
      next = TimeObsListPtr(new TimeObsList());
    }

    qApp->processEvents();

    while ( next ) {

      if ( next->empty() ) {
	QMessageBox::critical( this, "Weather",
			       "Får ikke kontakt med kvalobs.\nKan ikke fortsette.",
  			       QMessageBox::Ok, QMessageBox::NoButton );

      	return;
      }


      TimeObsListPtr current = next;
      next.reset();

      it = it->itemBelow();
      std::auto_ptr< boost::thread >  thread;
      if ( it ) {
    	ss = dynamic_cast<MSSListItem *>( it );
    	assert( ss );
    	d = & ss->data;
    	dates = dates_( d->obstime());
    	thread = thread_getTimeObs( next, d->stationID(), dates.first, dates.second );
      }

      delete stations->firstChild();
      WeatherDialog * wdlg = new WeatherDialog( current, type, sensor, reinserter, this );
      wdlg->resize(1200,700);
      wdlg->exec();

      qApp->processEvents();

      if ( thread.get() )
    	thread->join();
    }
  }

  void MultiStationSelection::keyPressEvent( QKeyEvent * e )
  {
    if ( e->key() == Qt::Key_Delete )
      delete stations->currentItem();
    else
      QDialog::keyPressEvent( e );
  }
}
