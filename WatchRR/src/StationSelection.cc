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
#include "StationSelection.h"
#include "BusyIndicator.h"
#include <qlineedit.h>
#include <q3datetimeedit.h>
#include <qlabel.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <kvalobs/kvData.h>
#include <kvcpp/KvApp.h>
#include <iostream>
#include <cassert>


using kvalobs::kvData;
using miutil::miDate;


namespace WatchRR
{
  StationSelection::TypeFromStation StationSelection::typeFromStation_;

  StationSelection::StationSelection( QWidget * parent, const kvData * data_ )
    : QWidget( parent )
  {
    std::cerr << "Dagens dato: " << miutil::miDate::today() << std::endl;

    kvData data;
    if ( data_ != 0 )
      data = * data_;
    else
      data.set( 0, miutil::miTime( miDate::today(), miutil::miClock(6,0,0) ),	0, 110, miutil::miTime(), 0, 0, 0, 0,	kvalobs::kvControlInfo(), kvalobs::kvUseInfo(), "" );

    Q3GridLayout * layout = new Q3GridLayout( this, 5, 2 );
    int row = 0;

    // Station:
    station_ =
      new QLineEdit( QString::number( data.stationID() ), "0000000", this );
    layout->addWidget( station_, row, 1 );
    layout->addWidget( new QLabel( station_, "&Stasjon", this ), row++, 0 );

    // Obstime:
    miutil::miDate d = data.obstime().date();
    obstime_ =
      new Q3DateEdit( QDate( d.year(), d.month(), d.day() ), this );
    obstime_->setOrder( Q3DateEdit::DMY ); // Norwegian standard
    layout->addWidget( obstime_, row, 1 );
    layout->addWidget( new QLabel( obstime_, "&Tid:", this ), row++, 0 );

    // TypeID:
    typeID_ =
      new QLineEdit( QString::number( data.typeID() ), "#000", this );
    layout->addWidget( typeID_, row, 1 );
    layout->addWidget( new QLabel( typeID_, "T&ype:", this ), row++, 0 );
    if ( ! data.typeID() )
      typeID_->setText( "302" );

    // Sensor:
    int snsr = data.sensor();
    if ( snsr >= '0' )
      snsr -= '0';
    sensor_ =
      new QLineEdit( QString::number( snsr ), "0", this );
    layout->addWidget( sensor_, row, 1 );
    layout->addWidget( new QLabel( sensor_, "S&ensor:", this ), row++, 0 );

    // Level:
    level_ =
      new QLineEdit( QString::number( data.level() ), "0", this );
    layout->addWidget( level_, row, 1 );
    layout->addWidget( new QLabel( level_, "&Level:", this ), row++, 0 );

    if ( typeFromStation_.empty() )
      setupTypeFromStation_();

    connect( station_, SIGNAL( textChanged(const QString &) ), this, SLOT( updateTypeID_() ) );
    connect( obstime_, SIGNAL( valueChanged(const QDate &) ), this, SLOT( updateTypeID_() ) );

    if ( ! data.stationID() ) {
      // Stations with RR_24 and typeid 302:
      int value[21] = {
        100, 250, 420, 700, 1230, 2910, 4740, 5350, 7660, 8720, 11710, 11900,
        12520, 13050, 13140, 13700, 15480, 16270, 17780, 18030, 18500
      };
      miutil::miClock tmp = miutil::miClock::oclock();
      std::srand( tmp.hour() * tmp.min() * tmp.sec() );
      int index = std::rand() % 21;
      typeID_->setText( "302" );
      station_->setText( QString::number( value[ index ] ) );
    }


  }


  int StationSelection::station() const {
    return station_->text().toInt();
  }


  miDate StationSelection::obstime() const {
    QDate d = obstime_->date();
    return miDate( d.year(), d.month(), d.day() );
  }


  int StationSelection::typeID() const {
    return typeID_->text().toInt();
  }


  int StationSelection::sensor() const {
    return sensor_->text().toInt() + '0';
  }


  int StationSelection::level() const {
    return level_->text().toInt();
  }

  kvData StationSelection::getKvData() const
  {
    kvData ret( station(), miutil::miTime(obstime(), miutil::miClock(6,0,0) ),
		0, 110, miutil::miTime::nowTime(), typeID(), sensor(), level(),
		0, kvalobs::kvControlInfo(),kvalobs::kvUseInfo(), "" );
    return ret;
  }

  void StationSelection::updateTypeID_()
  {
    TypeFromStation::const_iterator find = typeFromStation_.find( station() );
    if ( find != typeFromStation_.end() )
      typeID_->setText( QString::number( find->second ) );
  }

  void StationSelection::setupTypeFromStation_()
  {
    BusyIndicator busy;
    assert( kvservice::KvApp::kvApp );
    typeFromStation_.clear();
    std::list<kvalobs::kvObsPgm> opgm;
    bool ok = kvservice::KvApp::kvApp->getKvObsPgm( opgm, std::list<long>(), false );
    if ( not ok )
      return; // Got no contact with kvalobs: return.
    std::cerr << obstime() << std::endl;
    for ( std::list<kvalobs::kvObsPgm>::const_iterator it = opgm.begin(); it != opgm.end(); ++ it ) {
      //      std::cerr << it->stationID() << "," << it->paramID() << "," << it->fromtime() << "," << it->totime() << "," << it->typeID() << std::endl;
      if ( it->paramID() == 110
	   and ( it->typeID() == 302 or it->typeID() == 402 )
	   and ( it->kl06() or it->kl07() or it->collector() )
	   and ( it->fromtime() <  miutil::miTime( miDate::today(), miutil::miClock(0,0,0) )
		 and miutil::miTime( miDate::today(), miutil::miClock(0,0,0) ) < it->totime() )
	   ) {
        typeFromStation_[ it->stationID() ] = it->typeID();
	std::cerr << it->stationID() << "," <<  typeFromStation_[ it->stationID() ] << std::endl;
      }
    }
  }
}
