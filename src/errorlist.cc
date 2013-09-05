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
#include "AnalyseErrors.hh"
#include "ErrorListTableModel.hh"
#include "BusyIndicator.h"
#include "FailDialog.h"
#include "Functors.hh"
#include "hqcmain.h"
#include "hqc_paths.hh"
#include "hqc_utilities.hh"
#include "KvMetaDataBuffer.hh"

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

void ErrorList::fillMemoryStores(const std::vector<int>& selectedParameters,
                                 const timeutil::ptime& stime,
                                 const timeutil::ptime& etime,
                                 int lity,
                                 model::KvalobsDataListPtr dtl,
                                 const std::vector<modDatl>& mdtl)
{
    LOG_SCOPE("ErrorList");
    memStore2 = Errors::fillMemoryStore2(selectedParameters, TimeRange(stime, etime),
                                         (lity == erSa or lity == alSa), dtl, mdtl);
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

    fillMemoryStores(selectedParameters, stime, etime, lity, dtl, mdtl);

    std::vector<mem> errorList;
    BOOST_FOREACH(const mem& mo, memStore2) {
        if (mo.flg <= 1 && mo.flg > -3)
            continue;
        if (mo.controlinfo.flag(kvalobs::flag::fr) == 6 && mo.controlinfo.flag(kvalobs::flag::ftime) == 1)
            continue;
        errorList.push_back(mo);
    }
    mTableModel = std::auto_ptr<ErrorListTableModel>(new ErrorListTableModel(errorList));
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

void ErrorList::signalStationSelected()
{
    const int row = getSelectedRow();
    if (row < 0 or row == mLastSelectedRow)
        return;
    mLastSelectedRow = row;

    /*emit*/ signalNavigateTo(getKvData(row));
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
