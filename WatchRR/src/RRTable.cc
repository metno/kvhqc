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
#include "RRTable.h"
#include "enums.h"
#include "RRTableToolTip.h"
#include "RR_24DataTableItem.h"
#include "VxKvDataTableItem.h"
#include "SADataTableItem.h"
#include "SDDataTableItem.h"
#include "FDCheckTableItem.h"
#include "OkCheckTableItem.h"
#include "ControlFlagCell.h"
#include "BusyIndicator.h"
#include <algorithm>
#include <cmath>
#include <kvcpp/KvApp.h>
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvModelData.h>
#include <sstream>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <boost/assign/std/vector.hpp>

#include <cassert>

#include <iostream>

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace boost::assign;
using namespace WatchRR::cell;

namespace WatchRR
{
  static QString Days[7] = { "Sø", "Ma", "Ti", "On", "To", "Fr", "Lø" };

  enum Section {
    RR_24_orig, RR_24_corr, RR_24_flag, RR_24_model,
    SA_orig, SA_corr, SA_flag,
    SD_orig, SD_corr, SD_flag,
    V4_12, V5_12, V6_12,
    V4_18, V5_18, V6_18,
    V4_06, V5_06, V6_06,
    DateField, Collected, ObservationOk,
    NUM_SECTIONS
  };
  const QString headerStore[ NUM_SECTIONS ] =
    {
      "RR_24\norig", "RR_24\nkorr", "RR_24\nflagg", "RR_24\nmodell",
      "SA\norig", "SA\nkorr", "SA\nflagg",
      "SD\norig", "SD\nkorr", "SD\nflagg",
      "V4(12)", "V5(12)", "V6(12)",
      "V4(18)", "V5(18)", "V6(18)",
      "V4(06)", "V5(06)", "V6(06)",
      "Dato", "C", "OK"
    };

  void RRTable::setup()
  {
    setupTable();

    connect( this, SIGNAL(valueChanged(int,int)), SLOT(updateStatusbar(int,int)));
    connect( this, SIGNAL(currentChanged(int,int)), SLOT(updateStatusbar(int,int)));
  }

  RRTable::RRTable( int station, const miDate &date,
                    int type, int sensor, int level,
                    QWidget *parent, const char *name )
      : Q3Table( parent, name )
      , station( station )
      , refDate( date )
      , dateRange( calculateDateRange( getRefDate() ) )
      , type( type ), sensor( sensor ), level( level )
  {
    setup();

    cerr << "Dates (from - to):" << endl
    << getDateRange().first  << " - "
    << getDateRange().second << endl;

    BusyIndicator busy;
    observations = getDayObs( getStation(), getType(), getSensor(), getLevel(),
                              getDateRange().first, getDateRange().second );
    displayData();
  }

  RRTable::DateRange getDateRange_( std::vector<DayObs> * dayobs )
  {
    assert( dayobs->begin() != dayobs->end() );
    miDate start = dayobs->begin()->getDate();
    miDate end = dayobs->rbegin()->getDate();
    return RRTable::DateRange( start, end );
  }

  RRTable::RRTable( DayObsListPtr dayobs,
                    QWidget *parent, const char *name )
      : Q3Table( parent, name )
      , observations( dayobs )
      , station( (*dayobs)[0].getStation() )
      , refDate( (*dayobs)[0].getDate() )
      , dateRange( getDateRange_( &* dayobs ) )
      , type( (*dayobs)[0].getType () )
      , sensor( (*dayobs)[0].getSensor () )
      , level( (*dayobs)[0].getLevel () )
  {
    setup();
    displayData();
  }

  RRTable::~RRTable( )
  {}

  float interpretOrig_( const kvalobs::kvData & d )
  {
    if ( kvalobs::original_missing( d ) )
      return CellValueProvider::missing;
    else
      return d.original();
  }

  namespace
  {
    inline KvDataProvider::Data getVx_( int param, const miClock & clock, DayObs & dobs )
    {
      KvDataProvider::Data ret;
      ret.push_back( & dobs.get( param, clock ) );
      ret.push_back( & dobs.get( param +1, clock ) );
      return ret;
    }
  }

  namespace
  {
    inline float getModelRR_( const DayObs & dobs )
    {
      float ret = dobs.getModelRR();
      if ( ret == std::numeric_limits<float>().min() )
        return CellValueProvider::missing;
      return ret;
    }
  }

  void RRTable::displayData()
  {
    cerr << "RRTable::displayData()\n";

    int rows = observations->size();
    setNumRows( rows );
    for ( int i = 0; i < rows; i++ )
    {
      DayObs & dobs = (*observations)[i];
      kvData & d = dobs.get( RR_24 );
      RR_24DataTableItem * rr24_ = new RR_24DataTableItem( this, d );
      setItem( i, toCol[RR_24_orig], new RR_24DataTableItem( this, interpretOrig_( d ) ) );
      setItem( i, toCol[RR_24_corr], rr24_ );
      setItem( i, toCol[RR_24_flag], new ControlFlagCell( this, d ) );
      setItem( i, toCol[RR_24_model], new RR_24DataTableItem( this, getModelRR_( dobs ), "modellverdi" ) );
      setItem( i, toCol[ Collected ], new FDCheckTableItem( this, d ) );

      kvData & sa = dobs.get(SA, miClock(6,0,0));
      setItem( i, toCol[SA_orig], new snow::SADataTableItem( this, interpretOrig_( sa ) ) );
      setItem( i, toCol[SA_corr], new snow::SADataTableItem( this, sa ) );
      setItem( i, toCol[SA_flag], new ControlFlagCell( this, sa ) );

      kvData & sd = dobs.get(SD, miClock(6,0,0));
      setItem( i, toCol[SD_orig], new snow::SDDataTableItem( this, interpretOrig_( sd ) ) );
      setItem( i, toCol[SD_corr], new snow::SDDataTableItem( this, sd ) );
      setItem( i, toCol[SD_flag], new ControlFlagCell( this, sd ) );

      miClock clock( 12,0,0 ); // 12:00 - Day before
      KvDataProvider::Data dv4;
      setItem( i, toCol[V4_12], new VxKvDataTableItem( this, getVx_( V4, clock, dobs ) ) );
      setItem( i, toCol[V5_12], new VxKvDataTableItem( this, getVx_( V5, clock, dobs ) ) );
      setItem( i, toCol[V6_12], new VxKvDataTableItem( this, getVx_( V6, clock, dobs ) ) );
      clock.addHour( 6 );      // 18:00 - Day before
      setItem( i, toCol[V4_18], new VxKvDataTableItem( this, getVx_( V4, clock, dobs ) ) );
      setItem( i, toCol[V5_18], new VxKvDataTableItem( this, getVx_( V5, clock, dobs ) ) );
      setItem( i, toCol[V6_18], new VxKvDataTableItem( this, getVx_( V6, clock, dobs ) ) );
      clock.addHour( 12 );    // 06:00 - Same day
      setItem( i, toCol[V4_06], new VxKvDataTableItem( this, getVx_( V4, clock, dobs ) ) );
      setItem( i, toCol[V5_06], new VxKvDataTableItem( this, getVx_( V5, clock, dobs ) ) );
      setItem( i, toCol[V6_06], new VxKvDataTableItem( (Q3Table*)this, getVx_( V6, clock, dobs ) ) );

      RRTableItem * dateitem = new RRTableItem( this );
      dateitem->setText( verticalHeaderText( dobs.getDate() ) );
      setItem( i, toCol[ DateField ], dateitem );

      list<kvalobs::kvData *> okdl;
      dobs.getAll( okdl );
      setItem( i, toCol[ ObservationOk ], new OkCheckTableItem( this, okdl ) );

      miTime time = d.obstime();
      miDate date = time.date();
      if ( date == getRefDate() )
        setCurrentCell( i, toCol[ RR_24_corr ] );
      if ( date.day() == miDate::Thursday )
        ensureCellVisible( i, 0 );
      date.addDay( -1 );
      verticalHeader()->setLabel( i, verticalHeaderText( date ) );

    }
    adjustColumn( toCol[ DateField ] );
  }

  inline bool modifiable( Q3TableItem * ti )
  {
    const RRTableItem * rr = dynamic_cast<RRTableItem *>( ti );
    return rr and not rr->readOnly();
  }

  void RRTable::activateNextCell()
  {
    int row = max( currentRow(), 0 );
    int col = max( currentColumn(), 0 );
    for ( int i = 0; i < 2; ++ i )
    {
      if ( i and ( ++ row == numRows() ) )
        row = 0;
      while ( ++ col < numCols() )
      {
        if ( modifiable( item( row, col ) ) )
        {
          setCurrentCell( row, col );
          return;
        }
      }
      col = -1;
    }
    Q3Table::activateNextCell();
  }

  RRTable::DateRange RRTable::calculateDateRange( const miDate & refDate, int daysToDisplay )
  {
    miDate stop = refDate;
    miDate::days day = (miDate::days) stop.dayOfWeek();
    int addDays = miDate::Friday - day;
    if ( addDays <= 0 )
      addDays += 7;
    else if ( addDays < 4 )
      addDays = 4;
    stop.addDay( addDays );
    miDate latestDisplayable = miDate::today();
    latestDisplayable.addDay( 2 );
    if ( stop > latestDisplayable )
      stop = latestDisplayable;
    miDate start = stop;
    start.addDay( - daysToDisplay );

    return DateRange( start, stop );
  }


  QString RRTable::verticalHeaderText( const miDate &date ) const
  {
    ostringstream s;
    s << Days[ date.dayOfWeek()].toStdString() << " "
    << date.day() << "/"
    << date.month();
    return QString::fromStdString(s.str());
  }

  QSize RRTable::sizeHint() const
  {
    if ( numRows() == 0 )
      return Q3Table::sizeHint();

    int frames = frameWidth() * 2;

    int contWidth       = contentsWidth();
    int headerWidth     = verticalHeader()->frameSize().width();
    int width = contWidth + headerWidth + frames;
    width += verticalScrollBar()->size().width();

    int contHeight      = contentsHeight();
    int headerHeight    = horizontalHeader()->frameSize().height();
    int height = contHeight + headerHeight + frames;

    return QSize( width, height );
  }

  QSizePolicy RRTable::sizePolicy() const
  {
    return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
  }

  void RRTable::headerMove( int, int fromIndex, int toIndex )
  {
    toSec.insert( toSec.begin() + toIndex, toSec[fromIndex] );
    if ( fromIndex < toIndex )
    {
      toSec.erase( toSec.begin() + fromIndex );
    }
    else
    {
      toSec.erase( toSec.begin() + fromIndex +1 );
    }

    for ( unsigned int i = 0; i < toSec.size(); i++ )
      toCol[ toSec[i] ] = i;
  }

  void RRTable::getModifiedData( DataConsistencyVerifier::DataSet & mod )
  {
    const int row_ = currentRow();
    const int col_ = currentColumn();
    if ( row_ >= 0 and col_ >= 0 )
      endEdit( row_, col_, true, false );

    for ( int r = 0; r < numRows(); ++ r )
    {
      for ( int c = 0; c < numCols(); ++ c )
      {
        DataConsistencyVerifier * dcv = dynamic_cast<DataConsistencyVerifier *>( item( r, toCol[c] ) );
        if ( dcv )
          dcv->getUpdatedList( mod );
      }
    }
  }

  bool RRTable::saveData( const DataReinserter<kvservice::KvApp> *ri )
  {
    if ( ! ri )
    {
      QMessageBox::critical( this, "HQC - nedbør",
                             "Du er ikke autorisert til å lagre data i kvalobs.",
                             QMessageBox::Ok,  Qt::NoButton );
      return false;
    }

    DataConsistencyVerifier::DataSet mod;
    getModifiedData( mod );

    if ( mod.empty() )
    {
      QMessageBox::information( this, "HQC - nedbør",
                                "Du har ingen endringer å lagre.",
                                QMessageBox::Ok );
      return true;
    }

    list<kvData> dl( mod.begin(), mod.end() );

    cerr << "Lagrer:" << endl
    << decodeutility::kvdataformatter::createString( dl ) << endl;
    CKvalObs::CDataSource::Result_var res;
    {
      BusyIndicator busy;
      res = ri->insert( dl );
    }
    if ( res->res == CKvalObs::CDataSource::OK )
    {
      QMessageBox::information( this, "HQC - nedbør",
                                QString( "Lagret " + QString::number( dl.size() ) + " parametre til kvalobs." ),
                                QMessageBox::Ok );
    }
    else
    {
      QMessageBox::warning( this, "HQC - nedbør",
                            QString( "Klarte ikke å lagre data!\n"
                                     "Feilmelding fra kvalobs var:\n") +
                            QString(res->message),
                            QMessageBox::Ok,  Qt::NoButton );
      return false;
    }

    for ( list<kvData>::const_iterator it = dl.begin(); it != dl.end(); ++ it )
    {
      miutil::miTime t = it->obstime();
      miutil::miDate d = t.date();
      if ( t.hour() > 7 )
        d.addDay();
      for ( std::vector<DayObs>::iterator dobs = observations->begin(); dobs != observations->end(); ++ dobs )
      {
        if ( dobs->getDate() == d )
        {
          kvData & data = dobs->get( it->paramID(), it->obstime().clock() );
          data = * it;
        }
      }
    }
    return true;
  }

  void RRTable::setupTable( )
  {
    setColumnOrder();
    const vector<QString> & headers = getHeaderOrder();

    setNumCols( NUM_SECTIONS );
    Q3Header *h = horizontalHeader();
    for ( int i = 0; i < NUM_SECTIONS; i++ )
    {
      h->setLabel( i, headers[i] );
      adjustColumn( i );
    }

    setVScrollBarMode( Q3ScrollView::AlwaysOn  );
    setHScrollBarMode( Q3ScrollView::Auto );

    setColumnMovingEnabled( true );
    connect( horizontalHeader(), SIGNAL( indexChange( int, int , int ) ),
             this, SLOT( headerMove( int, int, int ) ) );
  }

  void RRTable::setColumnOrder()
  {
    static vector<Section> columnOrder;

    toSec.reserve( NUM_SECTIONS );
    toCol.reserve( NUM_SECTIONS );

    if ( columnOrder.empty() )
    {
      columnOrder.reserve( NUM_SECTIONS );
      columnOrder +=
        V4_12, V5_12, V6_12,
        V4_18, V5_18, V6_18,
        V4_06, V5_06, V6_06,
        DateField,
        RR_24_corr, SA_corr, SD_corr,
        Collected,
        RR_24_orig, SA_orig, SD_orig,
        RR_24_flag, SA_flag, SD_flag,
        RR_24_model, ObservationOk;
    }
    for ( unsigned int i = 0; i < columnOrder.size(); ++ i )
      toSec.push_back( columnOrder[ i ] );

    assert( toSec.size() == (unsigned int) NUM_SECTIONS );

    for ( unsigned int i = 0; i < toSec.size(); i++ )
      toCol[ toSec[i] ] = i;
  }

  const vector<QString> & RRTable::getHeaderOrder() const
  {
    static vector<QString> headers( NUM_SECTIONS );
    for ( int i = 0; i < NUM_SECTIONS; ++ i )
    {
      headers[i] = headerStore[ toSec[ i ] ];
    }
    return headers;
  }


  void RRTable::updateStatusbar( int row, int col )
  {
    Q3TableItem *i = item( row, col );
    SelfExplainable *e = dynamic_cast<SelfExplainable*>( i );
    QString msg;
    if ( e )
      msg = e->explain();
    else if ( i )
      msg = i->text();
    else
      msg = "";
  }
}
