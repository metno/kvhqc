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
/*! \file errorlist.cc
 *  \brief Code for the ErrorList class.
 *
 *  Displays the error list.
 *
 */

#include "errorlist.h"
#include "ErrorListTableModel.hh"
#include "BusyIndicator.h"
#include "FailDialog.h"
#include "Functors.hh"
#include "hqcmain.h"
#include "hqc_paths.hh"
#include "hqc_utilities.hh"
#include "KvMetaDataBuffer.hh"
#include "missingtable.h"

#include <kvalobs/kvDataOperations.h>
#include <kvcpp/KvApp.h>

#include <QtGui/QCloseEvent>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QMessageBox>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/range.hpp>

#include <cassert>

#define NDEBUG
#include "debug.hh"

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

void dumpMemstore(const std::vector<ErrorList::mem>& DBGE(memstore), const char* DBGE(label))
{
#ifndef NDEBUG
    LOG_SCOPE("ErrorList");
    LOG4SCOPE_DEBUG("memory store " << label << " size = " << memstore.size());
    int i = -1;
    BOOST_FOREACH(const ErrorList::mem& mo, memstore) {
        LOG4SCOPE_DEBUG(std::setw(7) << (++i)
                        << std::setw(7) << mo.stnr << ' '
                        << std::setw(21) << mo.obstime
                        << std::setw(5) << mo.parNo
                        << std::setw(9) << mo.orig
                        << std::setw(9) << mo.corr
                        << std::setw(9) << mo.morig << "  "
                        << std::setw(5) << mo.flTyp.toStdString() << "  " << mo.flg << "  "
                        << mo.controlinfo << "  " << mo.cfailed);
    }
#endif
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
    : QTableView(parent)
    , mainWindow( getHqcMainWindow( parent ) )
    , mLastSelectedRow(-1)
{
    LOG_SCOPE("ErrorList");

    setCaption(tr("HQC - Error List"));
    setIcon( QPixmap( ::hqc::getPath(::hqc::IMAGEDIR) + "/hqc.png" ) );

    verticalHeader()->setDefaultSectionSize(20);
    verticalHeader()->hide();
    setSelectionBehavior(SelectRows);
    setSelectionMode(SingleSelection);
    setSortingEnabled(true);
    QSortFilterProxyModel* sortProxy = new QSortFilterProxyModel(this);
    setModel(sortProxy);
    
    QAction* lackListAction = new QAction(tr("&Missing list"), this);
    lackListAction->setShortcut(tr("Ctrl+M"));
    connect(lackListAction, SIGNAL(activated()), this, SLOT(setupMissingList()));
    addAction(lackListAction);
    
    connect(mainWindow, SIGNAL(windowClose()), this, SIGNAL(errorListClosed()));
    connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(showFail(const QModelIndex&)));
    
    makeErrorList(selectedParameters, stime, etime, lity, dtl, mdtl);
    sortProxy->setSourceModel(mTableModel.get());

    connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

    horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_STATION_ID,    50);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_STATION_NAME,  80);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_MONTH,     30);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_DAY,       30);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_HOUR,      30);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_MINUTE,    30);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_PARAM,     50);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_TYPEID,    40);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_ORIG,      60);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_CORR,      60);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_MODEL,     60);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_FLAG_NAME, 50);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_FLAG_EQ,   15);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_FLAG_VAL,  30);
}

ErrorList::~ErrorList()
{
}

void ErrorList::makeMissingList(const std::vector<int>& selectedParameters,
                                const timeutil::ptime& stime,
                                const timeutil::ptime& etime,
                                model::KvalobsDataListPtr dtl)
{
    LOG_SCOPE("ErrorList");
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
            if (data.otime() < stime || data.otime() > etime)
                continue;
            if (not specialTimeFilter(parameterID, data.otime()))
                continue;
            if (not typeFilter(data.stnr(), parameterID, data.typeId(parameterID), data.otime()))
                continue;
            
            const int fnum = data.controlinfo(parameterID).flag(kvalobs::flag::fnum);
            if( fnum == 6 ) {
                const int tdiff = timeutil::hourDiff(data.otime(),stime);
                missObs mobs;
                mobs.oTime = data.otime();
                mobs.parno  = parameterID;
                mobs.statno = data.stnr();
                mobs.missNo = missCount;
                if (tdiff - prevTime != 1 || mobs.parno != prevPara || mobs.statno != prevStat) {
                    missCount = 0;
                }
                if (tdiff - prevTime == 1 && mobs.parno == prevPara && mobs.statno == prevStat && missCount > 4) {
                    LOG4SCOPE_DEBUG(DBG1(mobs.oTime) << DBG1(mobs.statno) << DBG1(parameterID) << DBG1(missCount));
                    mList.push_back(mobs);
                }
                missCount++;
                prevTime = tdiff;
                prevStat = mobs.statno;
                prevPara = mobs.parno;
            }
        }
    }
}

bool ErrorList::obsInMissList(const mem& memO)
{
    BOOST_FOREACH(const missObs& miss, mList) {
        if( miss.statno == memO.stnr && miss.parno == memO.parNo && miss.oTime == memO.obstime ) {
            LOG4HQC_DEBUG("ErrorList", "obs in miss list");
            return true;
        }
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
    LOG_SCOPE("ErrorList");
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
            LOG4SCOPE_DEBUG(DBG1(memObs.obstime) << DBG1(memObs.stnr) << DBG1(parameterID));
            if (not specialTimeFilter(parameterID, data.otime()))
                continue;
            if (not typeFilter(data.stnr(), parameterID, data.typeId(parameterID), data.otime()))
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
            LOG4SCOPE_DEBUG(DBG1(flg) << DBG1(flTyp));
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
                           ((flg == 2 || flg == 3 || flg == 0xA) && flTyp == "fw") ||
                           (flg > 0 && flTyp == "fmis" ) ||
                           (flg == 7 && flTyp == "fd") )
                {
                    memStore2.push_back(memObs);
                }
            } else if (lity == erSa or lity == alSa) {
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

void ErrorList::makeErrorList(const std::vector<int>& selectedParameters,
                              const timeutil::ptime& stime,
                              const timeutil::ptime& etime,
                              int lity,
                              model::KvalobsDataListPtr dtl,
                              const std::vector<modDatl>& mdtl)
{
    LOG_SCOPE("ErrorList");
    BusyIndicator busyIndicator;

    makeMissingList(selectedParameters, stime, etime, dtl);
    fillMemoryStores(selectedParameters, stime, etime, lity, dtl, mdtl);

    std::vector<mem> errorList;
    BOOST_FOREACH(const mem& mo, memStore2) {
        if( mo.flg <= 1 && mo.flg > -3 )
            continue;
        if( mo.controlinfo.substr(kvalobs::flag::fr,1) == "6" && mo.controlinfo.substr(kvalobs::flag::ftime,1) == "1"  )
            continue;
        errorList.push_back(mo);
    }
    mTableModel = std::auto_ptr<ErrorListTableModel>(new ErrorListTableModel(errorList));
}

bool ErrorList::priorityParameterFilter(int parNo)
{
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

void ErrorList::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    LOG_SCOPE("ErrorList");
    showSameStation();
    signalStationSelected();
}

void ErrorList::showFail(const QModelIndex& index)
{
    if (index.column() < 10 or index.column() > 12)
        return;
    FailInfo::FailDialog fDlg;
    fDlg.failList->newData(getKvData(index.row()));
    fDlg.exec();
}

int ErrorList::getSelectedRow() const
{
    QModelIndexList selectedRows = selectionModel()->selectedRows();
    if (selectedRows.size() != 1)
        return -1;
    const QModelIndex indexProxy = selectedRows.at(0);
    const QModelIndex indexModel = static_cast<QSortFilterProxyModel*>(model())->mapToSource(indexProxy);
    return indexModel.row();
}

void ErrorList::showSameStation()
{
    LOG_SCOPE("ErrorList");
    const int row = getSelectedRow();
    LOG4SCOPE_DEBUG(DBG1(row));
    if (row < 0)
        return;
    mTableModel->showSameStation(mTableModel->mem4Row(row).stnr);
}

/*!
 * \brief Update kvalobs, set hqc-flag = 2 for obs not in errorlist
 */
void ErrorList::updateKvBase(const mem& memStore)
{
    kvalobs::kvData kd = getKvData(memStore);

    //TODO: Remove next 3 lines when the new QC1-9 is ready
    kvControlInfo cif = kd.controlinfo();
    cif.set(kvalobs::flag::fhqc, 2);
    kd.controlinfo(cif);

    mainWindow->saveDataToKvalobs(kd);
}

void ErrorList::signalStationSelected()
{
    const int row = getSelectedRow();
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
    const int row = getSelectedRow();
    if (row < 0)
        return;

    const mem& m = getMem(row);
    if (m.controlinfo[kvalobs::flag::fnum] == '6')
        execMissingList();
}

const ErrorList::mem& ErrorList::getMem(int row) const
{
    return mTableModel->mem4Row(row);
}

kvData ErrorList::getKvData(const mem& m) const
{
    return kvData(m.stnr, timeutil::to_miTime(m.obstime), m.orig,
                  m.parNo, timeutil::to_miTime(m.tbtime),
                  m.typeId, m.sen, m.lev, m.corr, m.controlinfo,
                  m.useinfo, m.cfailed);
}

kvalobs::kvData ErrorList::getKvData() const
{
    const int row = getSelectedRow();
    if (row < 0)
        return kvData(0, timeutil::ptime(), -32767,
                      0, timeutil::ptime(), 0, 0, 0, -99999,
                      std::string("FFFFFFFFFFFFFFFF"),
                      std::string("FFFFFFFFFFFFFFFF"),
                      "no-row-in-errorlist");
    else
        return getKvData(getMem(row));
}

namespace /* anonymous */ {

bool set_no_accumulation(kvalobs::kvData& kd)
{
    kvalobs::kvControlInfo ci = kd.controlinfo();
    if (ci.flag(kvalobs::flag::fd) != 3)
        return false;
    ci.set(kvalobs::flag::fd, 1);
    kd.controlinfo(ci);
    return true;
}

void set_fhqc(kvalobs::kvData& kd, int fhqc)
{
    kvalobs::kvControlInfo ci = kd.controlinfo();
    ci.set(kvalobs::flag::fhqc, fhqc);
    kd.controlinfo(ci);
}

void set_fmis(kvalobs::kvData& kd, int fmis)
{
    kvalobs::kvControlInfo ci = kd.controlinfo();
    ci.set(kvalobs::flag::fmis, fmis);
    kd.controlinfo(ci);
}

} // anonymous namespace

void ErrorList::saveChanges()
{
    LOG_SCOPE("ErrorList");

    typedef std::list<kvData> kvDataList;
    kvDataList modData;
    BOOST_FOREACH(const mem& mo, mTableModel->errorList()) {
        if (mo.change == NO_CHANGE)
            continue;
        kvalobs::kvData kd = getKvData(mo);
        const kvalobs::kvControlInfo cif = kd.controlinfo();

        const int fmis = cif.flag(kvalobs::flag::fmis);
        const int fd   = cif.flag(kvalobs::flag::fd);
        bool qc2ok = mo.changed_qc2allowed;

        switch (mo.change) {
        case NO_CHANGE:
            continue;
        case CORR_OK:
            if (Helpers::float_eq()(kd.original(), kd.corrected())
                and (not Helpers::is_accumulation(fd)) and fmis < 2)
            {
                kvalobs::hqc::hqc_accept(kd);
                set_no_accumulation(kd);
            } else if (fmis == 0) {
                set_fhqc(kd, 7);
            } else if (fmis == 1) {
                set_fhqc(kd, 5);
            } else {
                LOG4SCOPE_ERROR("bad corr ok, would not set fhqc, kd=" << kd);
                continue;
            }
            break;
        case ORIG_OK:
            if (fmis == 3) {
                LOG4SCOPE_ERROR("fmis=3, orig ok not possible, should not have been here, kd=" << kd);
                continue;
            }
            if (cif.flag(kvalobs::flag::fnum) == 0 and not (fmis == 0 or fmis == 1 or fmis == 2)) {
                LOG4SCOPE_ERROR("bad orig ok, would not set fhqc, kd=" << kd);
                continue;
            }
            kd.corrected(kd.original());
            kvalobs::hqc::hqc_accept(kd);
            if (fmis == 0 or fmis == 2) {
                set_fmis(kd, 0);
                set_no_accumulation(kd);
            } else if (fmis == 1) {
                set_fmis(kd, 3);
            }
            break;
        case CORRECTED:
        case INTERPOLATED:
            if (Helpers::is_accumulation(fd)) {
                LOG4SCOPE_WARN("corr/interp for accumulation, should not have been here, kd=" << kd);
                continue;
            }
            kvalobs::hqc::hqc_auto_correct(kd, mo.changed_value);
            qc2ok = false;
            break;
        case REDISTRIBUTED:
            LOG4SCOPE_ERROR("should not make redistributions in error list, kd=" << kd);
            continue;
        case REJECTED:
            if (fmis == 1 or fmis == 3) {
                LOG4SCOPE_ERROR("fmis=1/3, cannot reject, should not have been here, kd=" << kd);
                continue;
            }
            kvalobs::hqc::hqc_reject(kd);
            break;
        }
        if (qc2ok)
            set_fhqc(kd, 4);

        modData.push_back( kd );
    }

    LOG4SCOPE_DEBUG("modData =\n" << decodeutility::kvdataformatter::createString(modData));

    if (modData.empty()) {
        QMessageBox::information(this,
                                 tr("No unsaved data"),
                                 tr("There are no unsaved data."),
                                 QMessageBox::Ok, Qt::NoButton);
        return;
    }

    if (mainWindow->saveDataToKvalobs(modData)) {
        QMessageBox::information(this,
                                 tr("Data saved"),
                                 tr("%1 rows have been saved to kvalobs. Warning: data shown in error "
                                    "and data list might no longer be consistent with kvalobs.").arg(modData.size()),
                                 QMessageBox::Ok, Qt::NoButton);
    }
}

bool ErrorList::maybeSave()
{
    bool modified = false;
    BOOST_FOREACH(const mem& mo, mTableModel->errorList()) {
        if (mo.change != NO_CHANGE) {
            modified = true;
            break;
        }
    }

    bool ret = true;
    if (modified) {
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

void ErrorList::closeEvent(QCloseEvent* event)
{
    if (maybeSave())
        QTableView::closeEvent(event);
    else
        event->ignore();
}
