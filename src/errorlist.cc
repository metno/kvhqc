/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2013 met.no

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
/*! \file errolist.cc
 *  \brief Code for the ErrorList class.
 *
 *  Displays the error list.
 *
 */
#define NDEBUG

#include "errorlist.h"
#include "BusyIndicator.h"
#include "ErrorListFirstCol.h"
#include "FailDialog.h"
#include "debug.hh"
#include "hqcmain.h"
#include "hqc_paths.hh"
#include "hqc_utilities.hh"
#include "KvMetaDataBuffer.hh"
#include "missingtable.h"

#include <qUtilities/miMessage.h>

#include <kvalobs/kvDataOperations.h>
#include <kvcpp/KvApp.h>

#include <QtCore/QFile>
#include <QtGui/QAction>
#include <QtGui/QCursor>
#include <QtGui/QHelpEvent>
#include <QtGui/QLineEdit>
#include <QtGui/QToolTip>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>
#include <Qt3Support/Q3TextEdit>
#include <Qt3Support/q3textstream.h>
#include <Qt3Support/Q3SimpleRichText>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/range.hpp>

#include <cassert>

using namespace kvalobs;

namespace /* anonymous */ {

const int  parNoControl[] = {
    2,    3,  4,  5,  6,  9, 10, 11, 12, 13,
    17,  20, 21, 22, 23, 24, 25, 26, 27, 28,
    44,  45, 46, 47, 48, 49, 50, 51, 52, 53,
    54,  55, 56, 57,101,102,103,115,116,124,
    138,191,192,193,194,195,196,197,198,199,
    202,226,227,229,230,231,232,233,234,235,
    236,237,238,239,240,241,247,261,271,272,
    274,275,276,277,278,279,280,281,282,283,
    284,285,286,287,288,289,290,291,292,293,
    294,295,296,297,298,299,300,301,302,303,
    304,305,306,307,308
};

const QString controlNoControl[] = {"QC1-2-100","QC1-2-123A","QC1-2-123B","QC1-2-123C","QC1-2-124A",
				    "QC1-2-125A","QC1-2-126B","QC1-2-129A","QC1-2-129B","QC1-2-130A",
				    "QC1-2-131","QC1-2-132","QC1-2-133","QC1-2-134B","QC1-2-139A",
				    "QC1-2-139B","QC1-2-142","QC1-2-143","QC1-2-144","QC1-2-145",
				    "QC1-2-146A","QC1-2-146B","QC1-2-146C","QC1-2-146D","QC1-2-147_148A",
				    "QC1-2-147_148B","QC1-2-147_148C","QC1-2-147_148D","QC1-2-149A",
				    "QC1-2-149B","QC1-2-149C","QC1-2-149D","QC1-2-150A","QC1-2-150B",
				    "QC1-2-150C","QC1-2-150D","QC1-2-151A","QC1-2-151B","QC1-2-151C",
				    "QC1-2-151D","QC1-2-152","QC1-2-153","QC1-2-154","QC1-2-155",
				    "QC1-2-156A","QC1-2-156B","QC1-2-156C","QC1-2-156D","QC1-2-158A",
				    "QC1-2-158B","QC1-2-158C","QC1-2-158D","QC1-2-159A","QC1-2-159B",
				    "QC1-2-159C","QC1-2-159D","QC1-2-160A","QC1-2-160B", "QC1-2-160C",
				    "QC1-2-160D","QC1-2-161A","QC1-2-161B","QC1-2-161C","QC1-2-161D",
				    "QC1-2-162A","QC1-2-162B","QC1-2-162C","QC1-2-162D","QC1-2-163A",
				    "QC1-2-163B","QC1-2-163C","QC1-2-163D","QC1-2-164","QC1-2-165",
				    "QC1-2-166","QC1-2-167","QC1-2-168","QC1-2-169","QC1-2-170",
				    "QC1-2-171","QC1-2-172","QC1-2-173","QC1-2-175"};

const float discardedValue_ = -32766.0;

void dumpMemstore(const std::vector<ErrorList::mem>& memstore, const char* label)
{
    std::cerr << "memory store " << label << " size = " << memstore.size() << std::endl;
    int i = -1;
    BOOST_FOREACH(const ErrorList::mem& mo, memstore) {
        std::cerr << std::setw(7) << (++i)
                  << std::setw(7) << mo.stnr
                  << std::setw(21) << mo.obstime
                  << std::setw(5) << mo.parNo
                  << std::setw(9) << std::setprecision(1) << mo.orig
                  << std::setw(9) << std::setprecision(1) << mo.corr
                  << std::setw(9) << std::setprecision(1) << mo.morig << "  "
                  << std::setw(5) << mo.flTyp.toStdString() << "  " << mo.flg << "  "
                  << mo.controlinfo << "  " << mo.cfailed << std::endl;
    }
    std::cerr << std::endl;
}

} /* anonymous namespace */

/*!
 * \brief Constructs the error list
 */
ErrorList::ErrorList(const std::vector<int>& selectedParameters,
		     const timeutil::ptime& stime,
		     const timeutil::ptime& etime,
		     QWidget* parent,
		     int lity,
		     model::KvalobsDataListPtr dtl,
		     const std::vector<modDatl>& mdtl)
    : Q3Table( 1000, 100, parent, "table")
    , mainWindow( getHqcMainWindow( parent ) )
    , mLastSelectedRow(-1)
{
    LOG_SCOPE();

    /*
      QMessageBox::information( this,
      tr("Feilliste"),
      tr("Bruk høyre musetast i \n\"Korrigert OK\",  \"Original OK\"  og"
      " \"Forkastet\"\nhvis QC2 skal kunne rette.\nVenstre musetast ellers."),
      QMessageBox::Ok,
      Qt::NoButton );
    */
    
    setVScrollBarMode( Q3ScrollView::AlwaysOn  );
    setMouseTracking(true);
    BusyIndicator busyIndicator;
    stationidCol = 1;
    typeidCol = 7;
    
    setCaption(tr("HQC - Error List"));
    setSorting(TRUE);
    readLimits();
    setSelectionMode( Q3Table::SingleRow );
    
    QAction* lackListAction = new QAction(tr("&Missing list"), this);
    lackListAction->setShortcut(tr("Ctrl+M"));
    
    fDlg = new FailInfo::FailDialog(this);
    
    connect( this, SIGNAL( clicked( int, int, int, const QPoint& ) ),
             SLOT( tableCellClicked( int, int, int) ) );
    
    connect( this, SIGNAL( valueChanged( int, int ) ),
             SLOT( markModified( int, int ) ) );
    
    connect( this, SIGNAL( valueChanged( int, int ) ),
             SLOT( clearOtherMods( int, int ) ) );
    
    connect( this, SIGNAL( doubleClicked( int, int, int, const QPoint&) ),
             this, SLOT( showFail(int, int, int, const QPoint&) ) );
    
    connect( this, SIGNAL( currentChanged(int, int) ),
             this, SLOT  ( updateFaillist(int, int) ) );
    
    connect( this, SIGNAL( currentChanged(int, int) ),
             this, SLOT  ( showSameStation()  ) );
    
    connect( this, SIGNAL( currentChanged(int, int) ),
             this, SLOT  ( signalStationSelected( int ) ) );
    
    connect( lackListAction, SIGNAL( activated() ),
             this, SLOT  ( setupMissingList() ) );
    
    connect( mainWindow, SIGNAL( windowClose() ),
             this, SIGNAL( errorListClosed() ) );
    
    std::cerr.setf(std::ios::fixed);

    setNumRows( 0 );
    setNumCols( 0 );
    setNumCols(21);
    setShowGrid(FALSE);
    verticalHeader()->hide();
    horizontalHeader()->setLabel(1, tr("Stnr"));
    horizontalHeader()->setLabel(2, tr("Navn"));
    horizontalHeader()->setLabel(3, tr("  Md"));
    horizontalHeader()->setLabel(4, tr("  Dg"));
    horizontalHeader()->setLabel(5, tr("  Kl"));
    horizontalHeader()->setLabel(6, tr("Para"));
    horizontalHeader()->setLabel(7, tr("Type"));
    horizontalHeader()->setLabel(8, tr("Orig.d"));
    horizontalHeader()->setLabel(9, tr("Corr.d"));
    horizontalHeader()->setLabel(10, tr("mod.v"));
    horizontalHeader()->setLabel(11, tr("Flag"));
    horizontalHeader()->setLabel(12, tr("="));
    horizontalHeader()->setLabel(13, tr("Fl.v"));
    horizontalHeader()->setLabel(14, tr("Corrected OK"));
    horizontalHeader()->setLabel(15, tr("Original OK"));
    horizontalHeader()->setLabel(16, tr("Interpolated"));
    horizontalHeader()->setLabel(17, tr("Redistributed"));
    horizontalHeader()->setLabel(18, tr("Corrected"));
    horizontalHeader()->setLabel(19, tr("Rejected"));
    horizontalHeader()->setLabel(20, "");
    
    makeMissingList(selectedParameters, stime, etime, dtl);
    fillMemoryStores(selectedParameters, stime, etime, lity, dtl, mdtl);
    
    setTableCells();

    showSameStation();
    
    setIcon( QPixmap( ::hqc::getPath(::hqc::IMAGEDIR) + "/hqc.png" ) );
    setCaption(tr("Error list"));
}

ErrorList::~ErrorList()
{
}

void ErrorList::makeMissingList(const std::vector<int>& selectedParameters,
                                const timeutil::ptime& stime,
                                const timeutil::ptime& etime,
                                model::KvalobsDataListPtr dtl)
{
    int missCount = 0;
    int prevTime = -12345678;
    int prevStat = -1;
    int prevPara = -1;
    BOOST_FOREACH(const int parameterID, selectedParameters) {
        BOOST_FOREACH(const model::KvalobsData& data, *dtl) {
            if( data.stnr() > 99999 )
                continue;
            //??
            //      if (  data.typeId(parameterID) < 0 )
            //        continue;
            //??
            if( data.otime() < stime || data.otime() > etime )
                continue;
            if( not specialTimeFilter(parameterID, data.otime()) )
                continue;
            if( not typeFilter(data.stnr(), parameterID, data.typeId(parameterID), data.otime()) )
                continue;
            
            const int fnum = data.controlinfo(parameterID).flag(kvalobs::flag::fnum);
            if( fnum == 6 ) {
                const int tdiff = timeutil::hourDiff(data.otime(),stime);
                missObs mobs;
                mobs.oTime = data.otime();
                mobs.time = tdiff;
                mobs.parno  = parameterID;
                mobs.statno = data.stnr();
                mobs.missNo = missCount;
                if ( mobs.time - prevTime != 1 || mobs.parno != prevPara || mobs.statno != prevStat  ) {
                    missCount = 0;
                }
                if ( mobs.time - prevTime == 1 && mobs.parno == prevPara && mobs.statno == prevStat && missCount > 4 ) {
                    mList.push_back(mobs);
                }
                missCount++;
                prevTime = mobs.time;
                prevStat = mobs.statno;
                prevPara = mobs.parno;
            }
        }
    }
}

bool ErrorList::obsInMissList(const mem& memO)
{
    BOOST_FOREACH(const missObs& miss, mList) {
        if( miss.statno == memO.stnr && miss.parno == memO.parNo && miss.oTime == memO.obstime )
            return true;
    }
    return false;
}

void ErrorList::fillMemoryStores(const std::vector<int>& selectedParameters,
                                 const timeutil::ptime& stime,
                                 const timeutil::ptime& etime,
                                 int lity,
                                 model::KvalobsDataListPtr dtl,
                                 const std::vector<modDatl>& mdtl)
{
    BOOST_FOREACH(const model::KvalobsData& data, *dtl) {
        if( data.stnr() > 99999 )
            continue;
#if 0
#warning Is showTypeId correct here? (It was a bug before checking if a pointer was less than zero)
        if( data.showTypeId() < 0 )
            continue;
#endif
        if( data.otime() < stime || data.otime() > etime )
            continue;
        mem memObs;
        memObs.obstime = data.otime();
        memObs.tbtime = data.tbtime();
        memObs.name = data.name();
        memObs.stnr = data.stnr();

        BOOST_FOREACH(const int parameterID, selectedParameters) {
            if( not specialTimeFilter(parameterID, data.otime()) )
                continue;
            if( not typeFilter(data.stnr(), parameterID, data.typeId(parameterID), data.otime()) )
                continue;
            memObs.typeId      = data.typeId(parameterID);
            memObs.orig        = data.orig(parameterID);
            memObs.corr        = data.corr(parameterID);
            memObs.sen         = data.sensor(parameterID);
            memObs.lev         = data.level(parameterID);
            memObs.controlinfo = data.controlinfo(parameterID).flagstring();
            memObs.useinfo     = data.useinfo(parameterID).flagstring();
            memObs.cfailed     = data.cfailed(parameterID);
            memObs.parNo       = parameterID;
            memObs.morig = -32767.0;

            if (KvMetaDataBuffer::instance()->isModelParam(memObs.parNo)) {
                BOOST_FOREACH(const modDatl& md, mdtl) {
                    if( md.stnr == memObs.stnr && md.otime == memObs.obstime ) {
                        memObs.morig = md.orig[memObs.parNo];
                    }
                }
            }

            if( obsInMissList(memObs) ) {
                missList.push_back(memObs);
                continue;
            }

            // priority filters for controls and parameters
            QString flTyp = "";
            int flg = errorFilter(parameterID,
                                  memObs.controlinfo,
                                  memObs.cfailed,
                                  flTyp);
            if( flg > -3 and flg <= 1 )
                continue;
            if( flg == -3 )
                flg = QString::fromStdString(memObs.controlinfo).mid(kvalobs::flag::fmis,1).toInt(0,16);
            memObs.flg = flg;
            memObs.flTyp = flTyp;

            // insert data into appropriate memory stores
            if ( lity == erLi || lity == alLi ) {
                if( ((flg == 2 || flg == 3) && flTyp == "fr" ) ||
                    (flg == 2 && (flTyp == "fcc" || flTyp == "fcp") ) ||
                    ((flg == 2 || flg == 3 ||flg == 4 || flg == 5) && flTyp == "fnum") ||
                    ((flg == 2 || flg == 3)&& flTyp == "fw") ||
                    ((flg == 2 || flg == 4 || flg == 5 ) && flTyp == "fs" ) )
                {
                    if( isErrorInMemstore1(memObs) ) {
                        updateKvBase(memObs);
                        memStore2.push_back(memObs);
                    } else {
                        memStore1.push_back(memObs);
                    }
                } else if (((flg == 4 || flg == 5 || flg == 6) && flTyp == "fr" ) ||
                           ((flg == 3 || flg == 4 || flg == 6 || flg == 7 || flg == 9 ||
                             flg == 0xA || flg == 0xB || flg == 0xD ) && flTyp == "fcc" ) ||
                           ((flg == 3 || flg == 4 || flg == 6 || flg == 7 ||
                             flg == 0xA || flg == 0xB ) && flTyp == "fcp" ) ||
                           ((flg == 3 || flg == 6 || flg == 8 || flg == 9)&& flTyp == "fs" ) ||
                           (flg == 6 && flTyp == "fnum") ||
                           (( flg == 3 || flg == 4 || flg == 6) && flTyp == "fpos") ||
                           ((flg == 2 || flg == 3) && flTyp == "ftime") ||
                           ((flg == 4 || flg == 5 || flg == 6) && flTyp == "fw") ||
                           (flg > 0 && flTyp == "fmis" ) ||
                           (flg == 7 && flTyp == "fd") )
                {
                    memStore2.push_back(memObs);
                }
            } else if ( lity == erSa ) {
                if( ((flg == 4 || flg == 5 || flg == 6) && flTyp == "fr" )
                    || (flg == 2 && flTyp == "fs") )
                {
                    memStore2.push_back(memObs);
                }
            }
        }
    }

    dumpMemstore(memStore1, "1");
    dumpMemstore(memStore2, "2");
}

void ErrorList::setTableCells()
{
    int insRow = 0;
    setNumRows( memStore2.size() );
    BOOST_FOREACH(const mem& mo, memStore2) {
        setRowReadOnly( insRow, false);
        setItem( insRow, 0, new ErrorListFirstCol( this, insRow ) );
        
        //std::cerr << decodeutility::kvdataformatter::createString( getKvData( mo ) ) << std::endl;
        
        if( mo.flg <= 1 && mo.flg > -3 )
            continue;
        
        if( mo.controlinfo.substr(kvalobs::flag::fr,1) == "6" && mo.controlinfo.substr(kvalobs::flag::ftime,1) == "1"  )
            continue;
        
        DataCell* snIt = new DataCell(this, Q3TableItem::Never,QString::number(mo.stnr));
        setItem(insRow,1,snIt);
        
        DataCell* naIt = new DataCell(this, Q3TableItem::Never,mo.name.left(8));
        setItem(insRow,2,naIt);
        
        const QString isoDate = QString::fromStdString(timeutil::to_iso_extended_string(mo.obstime));
        DataCell* moIt = new DataCell(this, Q3TableItem::Never,isoDate.mid(5,2));
        setItem(insRow,3,moIt);
        
        DataCell* dyIt = new DataCell(this, Q3TableItem::Never,isoDate.mid(8,2));
        setItem(insRow,4,dyIt);
        
        DataCell* clIt = new DataCell(this, Q3TableItem::Never,isoDate.mid(11,2));
        setItem(insRow,5,clIt);
        
        QString parName = "???";
        try {
            parName = QString::fromStdString(KvMetaDataBuffer::instance()->findParam(mo.parNo).name());
        } catch (std::runtime_error&) {
            // unknown parameter
        }
        DataCell* paIt = new DataCell(this, Q3TableItem::Never,parName);
        setItem(insRow,6,paIt);
        
        DataCell* tiIt = new DataCell(this, Q3TableItem::Never,QString::number(mo.typeId));
        setItem(insRow,7,tiIt);

        const bool isCodeParam = KvMetaDataBuffer::instance()->isCodeParam(mo.parNo);
        const int nDigits = isCodeParam ? 0 : 1;
        DataCell* ogIt = new DataCell(this, Q3TableItem::Never,QString::number(mo.orig,'f',nDigits));
        setItem(insRow,8,ogIt);
        
        DataCell* coIt = new DataCell(this, Q3TableItem::Never,QString::number(mo.corr,'f',nDigits));
        setItem(insRow,9,coIt);
        
        DataCell* mlIt = new DataCell(this, Q3TableItem::Never,QString::number(mo.morig,'f',nDigits));
        setItem(insRow,10,mlIt);
        
        DataCell* fiIt = new DataCell(this, Q3TableItem::Never,mo.flTyp);
        setItem(insRow,11,fiIt);
        
        DataCell* eqIt = new DataCell(this, Q3TableItem::Never,"=");
        setItem(insRow,12,eqIt);
        
        DataCell* fgIt = new DataCell(this, Q3TableItem::Never,QString::number(mo.flg));
        setItem(insRow,13,fgIt);
        
        insRow++;
    }
    
    std::cerr << "Antall rader = " << insRow << std::endl;
    
    for( int icol = 14; icol < 16; icol++ ) {
        for( int irow = 0; irow < insRow; irow++ ) {
            OkTableItem* okIt = new OkTableItem(this, "");
            setItem(irow,icol,okIt);
        }
    }
    
    for(int icol = 16; icol < 19; icol++ ) {
        for( int irow = 0; irow < insRow; irow++ ) {
            CrTableItem* crIt = new CrTableItem(this, Q3TableItem::WhenCurrent, "" ,// );
                                                icol != 14 and icol != 15 and icol != 19 );
            setItem(irow,icol,crIt);
        }
    }
    
    for( int irow = 0; irow < insRow; irow++ ) {
        OkTableItem* okIt = new OkTableItem(this, "");
        setItem(irow, 19, okIt);
    }

    for ( int icol = 0; icol < 14; icol++ )
        adjustColumn(icol);
    for ( int icol = 14; icol < 16; icol++ )
        setColumnWidth(icol, 90);
    for ( int icol = 16; icol < 20; icol++ )
        setColumnWidth(icol, 70);
    
    setColumnReadOnly ( 20, true );
    setColumnStretchable( 20, true );
}

void CrTableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected ) {
    Q3TableItem::paint( p, cg, cr, selected );
    p->drawRect( 0, 0, cr.width(), cr.height());
}

const QRegExp CrTableItem::re( "(\\-?[0-9]+(\\.[0-9])?)" );
const QRegExpValidator CrTableItem::validator( CrTableItem::re, NULL );
QWidget *CrTableItem::createEditor() const
{
    QLineEdit *le = dynamic_cast<QLineEdit *>( Q3TableItem::createEditor() );
    if ( numbers )
        le->setValidator( &validator );
    return le;
}


void OkTableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected ) {
    Q3CheckTableItem::paint( p, cg, cr, selected );
    p->drawRect( 0, 0, cr.width(), cr.height());
}


void DataCell::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected )
{
    p->setBrush(Qt::green);
    QColorGroup g( cg );
    g.setColor( QColorGroup::Background, Qt::green );
    Q3TableItem::paint( p, g, cr, selected );
}

bool ErrorList::priorityParameterFilter(int parNo) {
    if (std::binary_search(parNoControl, boost::end(parNoControl), parNo))
        return false;
    return (parNo < 1000);
}

int ErrorList::priorityControlFilter(QString cfailed)
{
    QStringList allFails = QStringList::split(",", cfailed, false);
    if( not allFails.isEmpty() ) {
        const QString* f = std::find(controlNoControl, boost::end(controlNoControl), allFails[0]);
        if( f != boost::end(controlNoControl) ) {
            if( allFails.count() == 1)
                return 0;
            else
                return -1;
        }
    }
    return 1;
}

int ErrorList::errorFilter(int parNo, std::string ctrInfo, std::string cFailed, QString& flTyp)
{
    QString flTypes[] = {"fagg","fr","fcc","fs","fnum","fpos","fmis","ftime","fw","fstat","fcp","fclim","fd","fpre","fcombi","fhqc"};
    QString qStrCtrInfo = QString::fromStdString(ctrInfo);
    QString cfailed = QString::fromStdString(cFailed);
    int flg = 0;
    int maxflg = -1;
    if( qStrCtrInfo.mid(kvalobs::flag::fpre,1).toInt(0,16) > 0 )
        return maxflg;
    if( !priorityParameterFilter(parNo) )
        return maxflg;
    const int pcf = priorityControlFilter(cfailed);
    if( pcf == 0 ) {
        return maxflg;
    } else if( pcf == -1 )
        maxflg = -2;
    // UNUSED int flInd = 0;
    for( int i = 0; i < 16; i++ ) {
        flg = qStrCtrInfo.mid(i,1).toInt(0,16);
        if( flg > 1 && maxflg > -2 ) {
            maxflg = flg;
            flTyp = flTypes[i];
            break;
        } else if( flg > 1 )
            maxflg = -1;
    }
    
    if( flTyp == "fmis" ) {
        maxflg = -3;
    }
    flg = qStrCtrInfo.mid(kvalobs::flag::fhqc,1).toInt(0,16);
    if( flg > 0 )//Hqc-flag allerede satt. Ikke ny kontroll
        maxflg = -1;
    return maxflg;
}

bool ErrorList::isErrorInMemstore1(const mem& mo)
{
    kvControlInfo cif(mo.controlinfo);
    const int fnum = cif.flag(kvalobs::flag::fnum), fw  = cif.flag(kvalobs::flag::fw);
    const int fs   = cif.flag(kvalobs::flag::fs),   fcp = cif.flag(kvalobs::flag::fcp);
    const int fr   = cif.flag(kvalobs::flag::fr),   fcc = cif.flag(kvalobs::flag::fcc);
    if( mo.flTyp == "fr" ) {
        if (KvMetaDataBuffer::instance()->isModelParam(mo.parNo)) {
            if ( fnum == 1 || (fnum > 1 && fw == 1) ) {
            } else if ( (fnum > 1 && fw > 1) || fw == 0 ) {
                return true;
            }
        } else {
            for( int k = 2; k < 16; k++ ) {
                //	  int iFlg = control.mid(k,1).toInt(0,16);
                int iFlg = cif.flag(k);
                if ( iFlg > 1 )
                    return true;
            }
        }
    } else if ( mo.flTyp == "fs" ) {
        if ( fs == 2 && fcp > 1 )
            return true;
        else if ( fs == 2 && fcp <= 1 ) {
            if ( fr == 1 && fw == 1 ) {
            } else if ( fr > 1 || fw > 1 ) {
                return true;
            }
        } else if ( fs == 4 && fcp > 1 ) {
            return true;
        } else if ( fs == 4 && fcp <= 1 && fr <= 1 && fw <= 1 ) {
        } else if ( fs == 5 && ( fr > 1 || fw > 1 ) ) {
            return true;
        } else if ( fs == 5 && fr <= 1 &&  fw <= 1  ) {
        }
    }
    //TODO: Proper treatment of fcc=2 and fcp=2
    /*
      else if ( mo.flTyp == "fcc" ) {
      if ( fcc == 2 )
      // find the other parameter
      error.push_back(i);
      }
      else if ( mo.flTyp == "fcp" ) {
      if ( fcp == 2 )
      // find the other parameter
      error.push_back(i);
      }
    */
    else if ( mo.flTyp == "fnum" ) {
        if ( mo.parNo == 177 || mo.parNo == 178 ) {
            return true;
        }
    } else if ( mo.flTyp == "fw" ) {
        if ( (fw == 2 || fw == 3) && ( fr > 1 || fcc > 1 || fs > 1 || fcp > 1) ) {
            return true;
        } else if ( (fw == 2 || fw == 3) && ( fr <= 1 && fcc <= 1 && fs <= 1 && fcp <= 1) ) {
        }
    } else {
    }
    return false;
}

void ErrorList::tableCellClicked(int row, int col, int /*button*/)
//void ErrorList::tableCellClicked(int row, int col, int button)
{
    if (col == 0 && row >= 0)
        selectRow(row);
    selectedRow = row;
    /*
      if ( button == Qt::RightButton && (col == 14 || col == 15 || col == 19) ){
      OkTableItem* okIt = static_cast<OkTableItem*>(item(row,col));
      okIt->setChecked(true);
      okIt->pressedButton = button;
      }
    */
}

bool ErrorList::specialTimeFilter( int par, const timeutil::ptime& otime)
{
    const int otime_hour = otime.time_of_day().hours();
    if ( ((par == 214 || par == 216) && !(otime_hour == 6 || otime_hour == 18))
         || (par == 112 && otime_hour != 6) )
    {
        return false;
    }
    return true;
}

bool ErrorList::typeFilter(int stnr, int par, int typeId, const timeutil::ptime& otime)
{
    const timeutil::pdate otime_date = otime.date();
    BOOST_FOREACH(const currentType& ct, mainWindow->getCurrentTypeList()) {
        if (stnr == ct.stnr && abs(typeId) == ct.cTypeId && par == ct.par && otime_date >= ct.fDate && otime_date <= ct.tDate )
            return true;
    }
    return false;
}

/*!
 * \brief Update kvalobs, set hqc-flag = 2 for obs not in errorlist
 */
void ErrorList::updateKvBase(const mem& memStore)
{
    if (mainWindow->getReinserter()) {
        kvData kd = getKvData( memStore );
        //TODO: Remove next 3 lines when the new QC1-9 is ready
        kvControlInfo cif = kd.controlinfo();
        cif.set(kvalobs::flag::fhqc,2);
        kd.controlinfo(cif);
        CKvalObs::CDataSource::Result_var result;
        {
            BusyIndicator busyIndicator;
            result = mainWindow->getReinserter()->insert(kd);
        }
        if ( result->res != CKvalObs::CDataSource::OK ) {
            std::cerr << "Could not send data!" << std::endl
                      << "Message was:" << std::endl
                      << result->message << std::endl;
            // TODO Handle Error!
            return;
        }
    }
}

void ErrorList::updateFaillist( int row, int /*col*/)
{
    if ( row >= 0 && row <  numRows() )
        fDlg->failList->newData( getKvData( row ) );
}

void ErrorList::showFail( int row, int col, int /*button*/, const QPoint& /*p*/)
{
    if ( (col > 10 && col < 14) && (row >= 0 && row <  numRows()) ) {
        fDlg->show();
    }
}

void ErrorList::swapRows( int row1, int row2, bool /*swapHeader*/ )
{
    Q3Table::swapRows( row1, row2, TRUE );
}

void ErrorList::sortColumn( int col, bool ascending, bool /*wholeRows*/ )
{
    /*emit*/ currentChanged( currentRow(), currentColumn() );
    Q3Table::sortColumn( col, ascending, TRUE );
    /*emit*/ currentChanged( currentRow(), currentColumn() );
    clearSelection( true );
    ensureCellVisible( currentRow(), 0 );
}

void ErrorList::showSameStation()
{
    int row = currentRow();
    if ( row < 0 )
        return;
    QString station = text( row, 1 );
    for ( int i = 0; i < numRows(); i++ ) {
        ErrorListFirstCol *elfc = dynamic_cast<ErrorListFirstCol*>(item( i, 0 ));
        if ( elfc != NULL ) {
            elfc->setSameStation( text( i, 1 ) == station );
            updateCell( i, 0 );
        }
    }
}

void ErrorList::markModified( int row, int col )
{
    ErrorListFirstCol * elfc = dynamic_cast<ErrorListFirstCol*>( item( row, 0) );
    assert( elfc );
    struct mem &msItem =
        memStore2[ elfc->memStoreIndex() ];

    kvData kd = getKvData( row );
    kvControlInfo cif = kd.controlinfo();


    bool OK = true;
    float cor;
    if ( col > 15 && col < 19 )
        cor = item(row, col)->text().toFloat(&OK);
    else if ( col == 15 ) {
        cor = msItem.orig;
    }
    else if ( col == 19 )
        cor = discardedValue_;
    else
        cor = msItem.corr;

    int panr = msItem.parNo;

    float uplim = highMap[panr];
    float downlim = lowMap[panr];
    if ( ( !text(row, col).isEmpty() && (cor > uplim || cor < downlim)) && col < 19 && col > 15) {
        QMessageBox::warning( this,
                              tr("Illegal value"),
                              tr("Value outside physical values"),
                              QMessageBox::Ok,
                              Qt::NoButton );
        item( row, col )->setText("");;
        return;
    }
    int fmis = cif.flag(kvalobs::flag::fmis);
    int fd = cif.flag(kvalobs::flag::fd);
    switch (col) {
    case 14:
    {
        if ( fmis == 0 ) {
            if ( fd == 2 || fd == 3 || fd > 5 ) {
                QMessageBox::information( this,
                                          tr("Wrong column"),
                                          tr("Accumulation. Use field 'redistributed'."),
                                          QMessageBox::Ok,
                                          Qt::NoButton );
                OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
                okIt->setChecked(false);
                item( row, col )->setText("");
                updateCell(row, col);
                return;
            }
        }
        else if ( fmis == 1 ) {
            if ( fd == 2 || fd > 5 ) {
                QMessageBox::information( this,
                                          tr("Wrong column"),
                                          tr("Accumulation. Use field &apos;redistributed&apos;."),
                                          QMessageBox::Ok,
                                          Qt::NoButton );
                OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
                okIt->setChecked(false);
                item( row, col )->setText("");
                updateCell(row, col);
                return;
            }
        }
        else if ( fmis == 2 ) {
            QMessageBox::information( this,
                                      tr("Wrong column"),
                                      tr("Corrected is missing. Use field 'Original OK' or 'Forkastet'."),
                                      QMessageBox::Ok,
                                      Qt::NoButton );
            OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
            okIt->setChecked(false);
            item( row, col )->setText("");
            updateCell(row, col);
            return;
        }
        else if ( fmis == 3 ) {
            QMessageBox::information( this,
                                      tr("Wrong column"),
                                      tr("Both original and corrected are missing. Use field 'Interpolated'."),
                                      QMessageBox::Ok,
                                      Qt::NoButton );
            OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
            okIt->setChecked(false);
            item( row, col )->setText("");
            updateCell(row, col);
            return;
        }
    }
    break;
    case 15:
    {
        if ( fmis == 1 ) {
            int dsc = QMessageBox::information( this,
                                                tr("Original missing"),
                                                tr("Do you want to set in -32767 as corrected value?"),
                                                tr("Yes"),
                                                tr("No") );
            if ( dsc == 1 ) { // Nei
                QMessageBox::information( this,
                                          tr("Wrong column"),
                                          tr("Use field 'Interpolated' if you want a new interpolated value, "
                                             "or 'Corrected OK' if you want to accept an existing value"),
                                          QMessageBox::Ok,
                                          Qt::NoButton );
                OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
                okIt->setChecked(false);
                item( row, col )->setText("");
                updateCell(row, col);
                return;
            }
        }
        else if ( fmis == 3 ) {
            OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
            okIt->setChecked(false);
            item( row, col )->setText("");
            updateCell(row, col);
            QMessageBox::information( this,
                                      tr("Wrong column"),
                                      tr("Both original and corrected value ar missing. Use field 'Interpolated'."),
                                      QMessageBox::Ok,
                                      Qt::NoButton );
            return;
        }
    }
    break;
    case 16:
    {
        if ( fd > 1 ) {
            QMessageBox::information( this,
                                      tr("Wrong column"),
                                      tr("Accumulation. Use field &apos;redistributed&apos;."),
                                      QMessageBox::Ok,
                                      Qt::NoButton );
            item( row, col )->setText("");
            return;
        }
    }
    break;
    case 17:
        break;
    case 18:
    {
        if ( fd >= 2 ) {
            QMessageBox::information( this,
                                      tr("Wrong column"),
                                      tr("Accumulation. Use field &apos;redistributed&apos;."),
                                      QMessageBox::Ok,
                                      Qt::NoButton );
            // UNUSED OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
            item( row, col )->setText("");
            //	updateCell(row, col);
            return;
        }
    }
    break;
    case 19:
    {
        if ( fmis == 1 ) {
            QMessageBox::information( this,
                                      tr("Wrong column"),
                                      tr("Cannot reject. Use field 'Original OK'."),
                                      QMessageBox::Ok,
                                      Qt::NoButton );
            OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
            okIt->setChecked(false);
            item( row, col )->setText("");
            updateCell(row, col);
            return;
        }
        else if ( fmis == 3 ) {
            QMessageBox::information( this,
                                      tr("Wrong column"),
                                      tr("Cannot reject. Use field 'Interpolert'."),
                                      QMessageBox::Ok,
                                      Qt::NoButton );
            OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
            okIt->setChecked(false);
            item( row, col )->setText("");
            updateCell(row, col);
            return;
        }
    }
    break;
    default:
        break;
    }
    //  std::cerr << "markModified( " << elfc->memStoreIndex() << ");\n";
    // Test starter
    bool emptyRec;
    if ( col == 14 || col == 15 || col == 19 ) {
        OkTableItem* okIt = static_cast<OkTableItem*>(item( row, col));
        emptyRec = !okIt->isChecked();
    }
    else
        emptyRec = text(row, col).isEmpty();
    if ( emptyRec )
        modifiedRows.erase( elfc );
    else
        //Test slutter
        modifiedRows.insert( elfc );
}

void ErrorList::clearOtherMods( int row, int col )
{
    std::cerr << "clearOtherMods( " << row << ", " << col << ")\n";

    if ( col > 13 && col < 20 ) {
        if ( (col > 15 && col < 19 && text( row, col ).stripWhiteSpace().isEmpty()) ) {
            item( row, col )->setText("");;
            return;
        }

        // Check if another column contains data
        for ( int icol = 14; icol < 16; icol++ ) {
            if ( icol != col ) {
                OkTableItem* okIt = static_cast<OkTableItem*>(item( row, icol));
                okIt->setChecked(false);
                updateCell(row, icol);
            }
        }

        for ( int icol = 16; icol < 19; icol++ ) {
            if ( icol != col ) {
                if ( not text(row, icol).isEmpty() ) {
                    item( row, icol )->setText("");

                    updateCell(row, icol);
                }
            }
        }
        if ( col != 19 ) {
            OkTableItem* okIt = static_cast<OkTableItem*>(item( row, 19));
            okIt->setChecked(false);
            updateCell(row, 19);
        }
    }
}

void ErrorList::signalStationSelected( int row )
{
    if (row < 0 or row == mLastSelectedRow)
        return;
    mLastSelectedRow = row;

    /*emit*/ signalNavigateTo(getKvData(row));
}

void ErrorList::execMissingList()
{
    BusyIndicator busy;
    if( not mList.empty() ) {
        MissingTable* mt = new MissingTable( 0, this);
        mt->show();
    } else {
        QMessageBox::information(this,
                                 tr("Missing list"),
                                 tr("Missing list does not contain more elements that shown in the error list"),
                                 tr("OK"));
    }
}

void ErrorList::setupMissingList()
{
    const struct mem *m = getMem( selectedRow );
    if ( m and m->controlinfo[4] == '6' ) {
        execMissingList();
    }
}

const struct ErrorList::mem *ErrorList::getMem(int row) const
{
    ErrorListFirstCol *elfc = dynamic_cast<ErrorListFirstCol*>(item(row, 0));
    if( !elfc )
        return 0;
    return &memStore2[ elfc->memStoreIndex() ];
}

kvData ErrorList::getKvData(const mem& m) const
{
    return kvData( m.stnr, timeutil::to_miTime(m.obstime), m.orig,
                   m.parNo, timeutil::to_miTime(m.tbtime),
                   m.typeId, m.sen, m.lev, m.corr, m.controlinfo,
                   m.useinfo, m.cfailed );
}

kvData ErrorList::getKvData(int row) const
{
    const struct mem *m = getMem( row );
    if( !m )
        return kvData();
    return getKvData( *m );
}

typedef std::list<kvData> kvDataList;

void ErrorList::saveChanges()
{
    std::cerr << "saving changes" << std::endl;

    DataReinserter<kvservice::KvApp> *reinserter = mainWindow->getReinserter();
    if (not reinserter) {
        QMessageBox::critical( this,
                               tr("Not authenticated"),
                               tr("You are not authenticated as operator. Cannot save sata."),
                               QMessageBox::Ok,
                               Qt::NoButton );
        return;
    }

    if (modifiedRows.empty()) {
        QMessageBox::information( this,
                                  tr("No unsaved data"),
                                  tr("There are no unsaved data."),
                                  QMessageBox::Ok,
                                  Qt::NoButton );
        return;
    }

    kvDataList modData;

    for ( CIModList it = modifiedRows.begin(); it != modifiedRows.end(); it++ ) {
        int row = (*it)->row();

        int ccol = 0;

        for ( int icol = 14; icol < 16; icol++ ) {
            OkTableItem* okIt = static_cast<OkTableItem*>(item( row, icol));
            if ( okIt->isChecked() ) {
                ccol = icol;
                break;
            }
        }

        for ( int icol = 16; icol < 19; icol++ ) {
            if ( not text(row, icol).isEmpty() ) {
                ccol = icol;
                break;
            }
        }

        OkTableItem* okIt = static_cast<OkTableItem*>(item( row, 19));
        if ( okIt->isChecked() )
            ccol = 19;

        kvData kd = getKvData( row );
        kvControlInfo cif = kd.controlinfo();
        kvUseInfo uif = kd.useinfo();
        std::cerr << "Gamle flagg = " << cif << " " << uif << std::endl;

        int fmis = cif.flag(kvalobs::flag::fmis);
        int fd = cif.flag(kvalobs::flag::fd);
        switch ( ccol ) {
        case 14:
        {
            OkTableItem* okIt = static_cast<OkTableItem*>(item( row, ccol));
            if ( kd.original() == kd.corrected() && ( fd == 0 || fd == 1 
                                                      || fd == 3 ) && fmis < 2 ) {
                //	  if ( okIt->pressedButton == Qt::RightButton )
                //	    cif.set(kvalobs::flag::fhqc,4);
                //	  else
                cif.set(kvalobs::flag::fhqc,1);
                if ( fd == 3 )
                    cif.set(kvalobs::flag::fd,1);
            }
            else {
                if ( fmis == 0 ) {
                    //	    if ( okIt->pressedButton == Qt::RightButton )
                    //	      cif.set(kvalobs::flag::fhqc,4);
                    //     	    else
                    cif.set(kvalobs::flag::fhqc,7);
                }
                else if ( fmis == 1 ) {
                    //	    if ( okIt->pressedButton == Qt::RightButton )
                    //	      cif.set(kvalobs::flag::fhqc,4);
                    //	    else
                    cif.set(kvalobs::flag::fhqc,5);
                }
            }
        }
        break;
        case 15:
        {
            OkTableItem* okIt = static_cast<OkTableItem*>(item( row, ccol));
            if ( cif.flag(kvalobs::flag::fnum) > 1 ) {
                //	  if ( okIt->pressedButton == Qt::RightButton )
                //	    cif.set(kvalobs::flag::fhqc,4);
                //	  else
                cif.set(kvalobs::flag::fhqc,1);
            }
            if ( fmis == 0 ) {
                //	  if ( okIt->pressedButton == Qt::RightButton )
                //	    cif.set(kvalobs::flag::fhqc,4);
                //	  else
                cif.set(kvalobs::flag::fhqc,1);
                kd.corrected(kd.original());
                if ( cif.flag(kvalobs::flag::fd) == 3 )
                    cif.set(kvalobs::flag::fd,1);
            }
            else if ( fmis == 1 ) {
                //	  if ( okIt->pressedButton == Qt::RightButton )
                //	    cif.set(kvalobs::flag::fhqc,4);
                //	  else
                cif.set(kvalobs::flag::fhqc,1);
                cif.set(kvalobs::flag::fmis,3);
                kd.corrected(kd.original());
            }
            else if ( fmis == 2 ) {
                //	  if ( okIt->pressedButton == Qt::RightButton )
                //	    cif.set(kvalobs::flag::fhqc,4);
                //	  else
                cif.set(kvalobs::flag::fhqc,1);
                cif.set(kvalobs::flag::fmis,0);
                kd.corrected(kd.original());
                if ( cif.flag(kvalobs::flag::fd) == 3 )
                    cif.set(kvalobs::flag::fd,1);
            }
            else if ( fmis == 3 ) {
                std::cerr << "Vi skulle ikke vært her" << std::endl;
            }
        }
        break;
        case 16:
        {
            if ( fmis == 0 || fmis == 2 ) {  //original exists, this is a correction
                //	  cif.set(kvalobs::flag::fmis,0);
                cif.set(kvalobs::flag::fmis,4);
                cif.set(kvalobs::flag::fhqc,7);
            }
            else if ( fmis == 1 || fmis == 3 ) {  //original is missing, this is an interpolation
                cif.set(kvalobs::flag::fmis,1);
                cif.set(kvalobs::flag::fhqc,5);
            }
        }
        break;
        case 17:
        {
            if ( fmis == 0 || fmis == 2 ) {
                //	if ( fmis == 0 ) {
                cif.set(kvalobs::flag::fmis,4);
            }
            else if ( fmis == 1 || fmis == 3 ) {
                cif.set(kvalobs::flag::fmis,1);
            }
            cif.set(kvalobs::flag::fd,2);
            cif.set(kvalobs::flag::fhqc,6);
        }
        break;
        case 18:
        {
            if ( fmis == 0 || fmis == 2 || fmis == 4 ) {  //original exists, this is a correction
                //	  cif.set(kvalobs::flag::fmis,0);
                cif.set(kvalobs::flag::fmis,4);
                cif.set(kvalobs::flag::fhqc,7);
            }
            else if ( fmis == 1 || fmis == 3 ) {  //original is missing, this is an interpolation
                cif.set(kvalobs::flag::fmis,1);
                cif.set(kvalobs::flag::fhqc,5);
            }
        }
        break;
        case 19:
        {
            int fmis = cif.flag(kvalobs::flag::fmis);
            if ( fmis == 1 )
                std::cerr << "VI SKULLE IKKE VÆRT HER!!!" << std::endl;
            else if ( fmis == 3 )
                std::cerr << "VI SKULLE IKKE VÆRT HER!!!" << std::endl;
            else {
                //	  if ( okIt->pressedButton == Qt::RightButton )
                //	    cif.set(kvalobs::flag::fhqc,4);
                //	  else
                cif.set(kvalobs::flag::fhqc,10);
                if ( fmis == 0 )
                    cif.set(kvalobs::flag::fmis,2);
            }
        }
        break;
        case 0:
            break;
        default:
            // Undo changes:
            std::cerr << "KNUT tester er vi her ???? " << std::endl;
            cif.set(kvalobs::flag::fhqc,0); break;
        }
        kd.controlinfo( cif );
        std::cerr << "Nye flagg    = " << cif << " " << uif << std::endl;

        //    if ( ccol > 14 and ccol < 19 )
        if ( ccol > 15 and ccol < 19 )
            kd.corrected( text( row, ccol ).toFloat() );
        else if ( ccol == 15 ) {
            // UNUSED const int tableOriginalValuePos = 8;
            // UNUSED float newCorrected = text( row, tableOriginalValuePos ).toFloat();
            kd.corrected( kd.original() );
        }
        else if ( ccol == 19 ) {
            kd.corrected( discardedValue_ );
        }
        else if ( ccol == 0 ) {
            const int tableOriginalValuePos = 9;
            // UNUSED float newCorrected = text( row, tableOriginalValuePos ).toFloat();
            kd.corrected( text( row, tableOriginalValuePos ).toFloat() );
        }

        modData.push_back( kd );
    }

    std::cerr << decodeutility::kvdataformatter::createString( modData ) << std::endl;


    CKvalObs::CDataSource::Result_var result;
    {
        BusyIndicator busyIndicator;
        result = reinserter->insert( modData );
    }

    if ( result->res != CKvalObs::CDataSource::OK ) {
        QMessageBox::critical( this,
                               tr("Cannot save data"),
                               tr("Cannot save data! Message from kvalobs was:\n%1")
                               .arg(QString(result->message)),
                               QMessageBox::Ok,
                               Qt::NoButton );
        return;
    }

    QMessageBox::information( this,
                              tr("Data saved"),
                              tr("%1 rows have been saved to kvalobs.").arg(modifiedRows.size()),
                              QMessageBox::Ok,
                              Qt::NoButton );

    modifiedRows.clear();
}

void ErrorList::readLimits()
{
    int par, dum;
    float low, high;
    QString limitsFile = ::hqc::getPath(::hqc::CONFDIR) + "/slimits";
    QFile limits(limitsFile);
    if ( !limits.open(QIODevice::ReadOnly) ) {
        std::cerr << "kan ikke åpne " << limitsFile.toStdString() << std::endl;
        exit(1);
    }
    Q3TextStream limitStream(&limits);
    while ( limitStream.atEnd() == 0 ) {
        limitStream >> par >> dum >> low >> high;
        lowMap[par] = low;
        highMap[par] = high;
    }
}

bool ErrorList::maybeSave()
{
    bool ret = true;
    if ( not modifiedRows.empty() ) {
        int result =
            QMessageBox::warning( this, tr("HQC"),
                                  tr("You have unsaved changes in the error list. Do you want to save them?"),
                                  tr("&Yes"), tr("&No"), tr("&Cancel"),
                                  0, 2 );
        if ( ! result )
            saveChanges();
        ret = result != 2;
    }
    return ret;
}

bool ErrorList::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QPoint cp = mapFromGlobal(helpEvent->globalPos());
        int cy = contentsY();
        int row = rowAt( cp.y() + cy )-1;
        // UNUSED int col = columnAt( cp.x() );

        QString cellText = text( row, stationidCol );
        if ( cellText.isNull() )
            return false;
        
        
        QString tipString;
        {
            bool ok = true;
            int stationID = cellText.toInt(&ok);
            if ( !ok ) // Could not convert cell contents to int.
                return false;
            tipString += Helpers::stationInfo(stationID);
        }

        cellText = text( row, typeidCol );
        if (cellText.isNull())
            return false;

        {
            bool ok = true;
            int typeID = cellText.toInt(&ok);
            if ( !ok ) // Could not convert cell contents to int.
                return false;

            tipString += Helpers::typeInfo(typeID);
        }
        QToolTip::showText(helpEvent->globalPos(), tipString);
    }
    return QWidget::event(event);
}

void ErrorList::closeEvent( QCloseEvent * event )
{
    if ( maybeSave() )
        Q3Table::closeEvent(event);
    else
        event->ignore();
}


QString DataCell::key() const
{
    QString item;
    if ( col() == 1 || col() == 3 || col() == 5 || col() == 7 || col() == 11) {
        item.sprintf("%08d",text().toInt());
    }
    else if ( col() == 8 || col() == 9 || col() == 10 ) {
        item.sprintf("%08.1f",text().toDouble()+33000);
    }
    else {
        item = text();
    }
    return item;
}
