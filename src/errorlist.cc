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
#include "Functors.hh"
#include "hqc_utilities.hh"
#include "KvMetaDataBuffer.hh"

#include <QtGui/QCloseEvent>
#include <QtGui/QHeaderView>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QMessageBox>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.ErrorList"
#include "HqcLogging.hh"

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
    METLIBS_LOG_SCOPE();

    verticalHeader()->setDefaultSectionSize(20);
    verticalHeader()->hide();
    setSelectionBehavior(SelectRows);
    setSelectionMode(SingleSelection);
    setSortingEnabled(true);
    setModel(mSortProxy.get());
    resizeHeaders();
    
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
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_STATION_NAME, 160);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_STATION_WMO,   45);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_MONTH,     30);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_DAY,       30);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_HOUR,      30);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_MINUTE,    30);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_PARAM,     60);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_TYPEID,    40);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_ORIG,      60);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_CORR,      60);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_MODEL,     60);
    horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_FLAGS,    120);
}

void ErrorList::setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits)
{
    METLIBS_LOG_SCOPE();
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
    METLIBS_LOG_SCOPE();
    showSameStation();
    signalStationSelected();
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
    METLIBS_LOG_SCOPE();
    const int row = getSelectedRow();
    METLIBS_LOG_DEBUG(LOGVAL(row));
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
