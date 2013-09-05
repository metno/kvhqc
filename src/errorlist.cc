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
#include "FailDialog.h"
#include "Functors.hh"
#include "hqc_utilities.hh"
#include "KvMetaDataBuffer.hh"

#include <QtGui/QCloseEvent>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QMessageBox>

#include <boost/foreach.hpp>

#define NDEBUG
#include "debug.hh"

using namespace kvalobs;

/*!
 * \brief Constructs the error list
 */
ErrorList::ErrorList(QWidget* parent)
    : QTableView(parent)
    , mLastSelectedRow(-1)
    , mErrorsForSalen(false)
    , mSortProxy(new QSortFilterProxyModel(this))
{
    LOG_SCOPE("ErrorList");

    verticalHeader()->setDefaultSectionSize(20);
    verticalHeader()->hide();
    setSelectionBehavior(SelectRows);
    setSelectionMode(SingleSelection);
    setSortingEnabled(true);
    setModel(mSortProxy.get());
    resizeHeaders();
    
    connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(showFail(const QModelIndex&)));
    
    connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

ErrorList::~ErrorList()
{
}

void ErrorList::setDataAccess(EditAccessPtr eda, ModelAccessPtr mda)
{
    DataView::setDataAccess(eda, mda);

    mTableModel = std::auto_ptr<ErrorListTableModel>(new ErrorListTableModel(eda, mda, Errors::Errors_t(), mErrorsForSalen));
    mSortProxy->setSourceModel(mTableModel.get());
    resizeHeaders();
}

void ErrorList::resizeHeaders()
{
    horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_STATION_ID,    60);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_STATION_NAME,  160);
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
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_FLAG_EQ,   20);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_FLAG_VAL,  50);
}

void ErrorList::setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits)
{
    LOG_SCOPE("ErrorList");
    DataView::setSensorsAndTimes(sensors, limits);

    mLastSelectedRow = -1;
    Errors::Errors_t memStore2;
    if (mDA)
        memStore2 = Errors::fillMemoryStore2(mDA, sensors, limits, mErrorsForSalen);

    mTableModel = std::auto_ptr<ErrorListTableModel>(new ErrorListTableModel(mDA, mMA, memStore2, mErrorsForSalen));
    mSortProxy->setSourceModel(mTableModel.get());
    resizeHeaders();
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

    LOG4SCOPE_DEBUG("sorry, FailDialog needs update");
    //// FIXME re-enable FailInfo
    // FailInfo::FailDialog fDlg;
    // fDlg.failList->newData(getKvData(index.row()));
    // fDlg.exec();
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
    EditDataPtr obs = mTableModel->mem4Row(row);
    int stationId = obs ? obs->sensorTime().sensor.stationId : -1;
    mTableModel->showSameStation(stationId);
}

void ErrorList::signalStationSelected()
{
    const int row = getSelectedRow();
    if (row < 0 or row == mLastSelectedRow)
        return;
    mLastSelectedRow = row;

    EditDataPtr obs = getObs(row);
    /*emit*/ signalNavigateTo(obs->sensorTime());
}

EditDataPtr ErrorList::getObs(int row) const
{
    return mTableModel->mem4Row(row);
}

EditDataPtr ErrorList::getObs() const
{
    return getObs(getSelectedRow());
}
