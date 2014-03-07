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

#include "ErrorList.hh"

#include "ErrorListTableModel.hh"
#include "common/AnalyseErrors.hh"
#include "common/Functors.hh"
#include "common/KvMetaDataBuffer.hh"
#include "util/Blocker.hh"

#include <QtGui/QCloseEvent>
#include <QtGui/QHeaderView>
#include <QtGui/QMessageBox>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.ErrorList"
#include "common/ObsLogging.hh"

using namespace kvalobs;

ErrorList::ErrorList(QWidget* parent)
  : QTableView(parent)
  , mBlockNavigateTo(0)
  , mErrorsForSalen(false)
{
  METLIBS_LOG_SCOPE();

  verticalHeader()->setDefaultSectionSize(20);
  verticalHeader()->hide();
  setSelectionBehavior(SelectRows);
  setSelectionMode(SingleSelection);
  resizeHeaders();
}

ErrorList::~ErrorList()
{
}

void ErrorList::setDataAccess(EditAccessPtr eda, ModelAccessPtr mda)
{
  DataView::setDataAccess(eda, mda);

  Blocker b(mBlockNavigateTo);
  mLastNavigated = SensorTime();
  mTableModel = std::auto_ptr<ErrorListTableModel>(new ErrorListTableModel(eda, mda,
          Errors::Sensors_t(), TimeRange(), mErrorsForSalen));
  setModel(mTableModel.get());
  connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
  resizeHeaders();
}

void ErrorList::resizeHeaders()
{
  horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  horizontalHeader()->resizeSection(ErrorListTableModel::COL_STATION_ID,    60);
  horizontalHeader()->resizeSection(ErrorListTableModel::COL_STATION_NAME, 160);
  horizontalHeader()->resizeSection(ErrorListTableModel::COL_STATION_WMO,   45);
  horizontalHeader()->resizeSection(ErrorListTableModel::COL_OBS_TIME,     150);
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

  Blocker b(mBlockNavigateTo);
  mLastNavigated = SensorTime();
  mTableModel = std::auto_ptr<ErrorListTableModel>(new ErrorListTableModel(mDA, mMA, sensors, limits, mErrorsForSalen));
  setModel(mTableModel.get());
  connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
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
  const QModelIndex indexModel = selectedRows.at(0);
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
  METLIBS_LOG_SCOPE();
  const int row = getSelectedRow();
  METLIBS_LOG_DEBUG(LOGVAL(row));
  if (row < 0)
    return;
  
  if (EditDataPtr obs = getObs(row)) {
    const SensorTime& st = obs->sensorTime();
    METLIBS_LOG_DEBUG(LOGVAL(st));
    if (st.valid() and not eq_SensorTime()(mLastNavigated, st)) {
      Blocker b(mBlockNavigateTo);
      mLastNavigated = st;
      METLIBS_LOG_DEBUG(LOGVAL(mBlockNavigateTo));
      if (b.open())
        /*emit*/ signalNavigateTo(st);
      return;
    }
  }

  // reach here if no obs or invalid => make mLastNavigated invalid
  mLastNavigated = SensorTime();
  METLIBS_LOG_DEBUG("invalidated mLastNavigated");
}

void ErrorList::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_SCOPE(LOGVAL(st) << LOGVAL(st.valid()) << LOGVAL(mLastNavigated));
  if ((not st.valid()) or eq_SensorTime()(mLastNavigated, st))
    return;
  mLastNavigated = st;

  const int row = mTableModel->findSensorTime(st);
  if (row < 0)
    return;

  Blocker b(mBlockNavigateTo);
  METLIBS_LOG_DEBUG(LOGVAL(mBlockNavigateTo));
  // scrollTo must come before select, otherwise scrolling will not happen
  scrollTo(mTableModel->index(row, 0));
  selectRow(row);
}

EditDataPtr ErrorList::getObs(int row) const
{
  return mTableModel->mem4Row(row);
}

EditDataPtr ErrorList::getObs() const
{
  return getObs(getSelectedRow());
}
