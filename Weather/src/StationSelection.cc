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
#include "KvMetaDataBuffer.hh"
#include "MiDateTimeEdit.hh"

#include <kvalobs/kvData.h>
#include <kvcpp/KvApp.h>

#include <QtGui/qlineedit.h>
#include <QtGui/qlabel.h>
#include <QtGui/qlayout.h>
#include <Qt3Support/Q3GridLayout>

#include <boost/foreach.hpp>

#include <iostream>
#include <cassert>

using kvalobs::kvData;

namespace Weather
{
  StationSelection::StationSelection( QWidget * parent, const kvData * data_ )
    : QWidget( parent )
  {
    std::cerr << "Dagens dato: " << timeutil::to_iso_extended_string(timeutil::now().date()) << std::endl;

    kvData data;
    if ( data_ != 0 )
      data = * data_;
    else
        data.set( 0, timeutil::to_miTime(timeutil::ptime(timeutil::now().date(), boost::posix_time::time_duration(6,0,0) )),
                0, 211, timeutil::to_miTime(timeutil::ptime()), 0, 0, 0, 0,       kvalobs::kvControlInfo(), kvalobs::kvUseInfo(), "" );

    Q3GridLayout * layout = new Q3GridLayout( this, 5, 2 );
    int row = 0;

    // Station:
    station_ =
      new QLineEdit( QString::number( data.stationID() ), "0000000", this );
    layout->addWidget( station_, row, 1 );
    layout->addWidget( new QLabel( station_, tr("&Stasjon"), this ), row++, 0 );

    // Obstime:
    timeutil::ptime d = timeutil::from_miTime(data.obstime());
    QDate dt = QDate(d.date().year(), d.date().month(), d.date().day());
    QTime ti = QTime(d.time_of_day().hours(), 0);
    obstime_ = new MiDateTimeEdit( QDateTime( dt, ti ), this );
    obstime_->setDisplayFormat("yyyy-MM-dd hh:mm");
    layout->addWidget( obstime_, row, 1 );
    layout->addWidget( new QLabel( obstime_, tr("&Tid:"), this ), row++, 0 );

    // TypeID:
    typeID_ = new QLineEdit( "", "#000", this );
    layout->addWidget( typeID_, row, 1 );
    layout->addWidget( new QLabel( typeID_, tr("T&ype:"), this ), row++, 0 );
    if ( ! data.typeID() )
      typeID_->setText( "" );

    // Sensor:
    int snsr = data.sensor();
    if ( snsr >= '0' )
      snsr -= '0';
    sensor_ =
      new QLineEdit( QString::number( snsr ), "0", this );
    layout->addWidget( sensor_, row, 1 );
    layout->addWidget( new QLabel( sensor_, tr("S&ensor:"), this ), row++, 0 );
    if ( ! data.sensor() )
      sensor_->setText( "" );

    connect( station_, SIGNAL( textChanged(const QString &) ), this, SLOT( updateTypeID_() ) );
  }


  int StationSelection::station() const {
    return station_->text().toInt();
  }


  timeutil::ptime StationSelection::obstime() const {
    QDateTime d = obstime_->dateTime();
    QDate dt = d.date();
    QTime ti = d.time();
    return timeutil::from_YMDhms(dt.year(), dt.month(), dt.day(), ti.hour(), 0, 0);
  }

  int StationSelection::typeID() const {
    return typeID_->text().toInt();
  }

  int StationSelection::sensor() const {
    return sensor_->text().toInt() + '0';
  }

  kvData StationSelection::getKvData() const
  {
    kvData ret( station(), timeutil::to_miTime(obstime()),
		0, 211, timeutil::to_miTime(timeutil::now()), typeID(), sensor(), 0,
		0, kvalobs::kvControlInfo(),kvalobs::kvUseInfo(), "" );
    return ret;
  }

  void StationSelection::updateTypeID_()
  {
      const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(station());
      BOOST_FOREACH(const kvalobs::kvObsPgm& op, obs_pgm) {
          if (op.paramID() == 110 /* RR_24 */
              and (op.typeID() == 302 or op.typeID() == 402 )
              and (op.kl06() or op.kl07()))
          {
              typeID_->setText(QString::number(op.typeID()));
          }
      }
  }

}
