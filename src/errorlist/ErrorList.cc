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
#include "ErrorListModel.hh"
#include "ErrorSearchDialog.hh"

#include "common/EditAccess.hh"
#include "common/Functors.hh"
#include "common/HqcApplication.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/ObsPgmRequest.hh"

#include <QtCore/QSettings>
#include <QtGui/QHeaderView>

#include <boost/foreach.hpp>

#include "ui_errorlist.h"

#define MILOGGER_CATEGORY "kvhqc.ErrorList"
#include "common/ObsLogging.hh"

using namespace kvalobs;

ErrorList::ErrorList(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui_ErrorList)
  , mDialog(new ErrorSearchDialog(this))
  , mDA(hqcApp->editAccess())
  , mMA(hqcApp->modelAccess())
  , mErrorsForSalen(false)
  , mObsPgmRequest(0)
{
  METLIBS_LOG_TIME();
  ui->setupUi(this);
  ui->tree->setSelectionBehavior(QTreeView::SelectRows);
  ui->tree->setSelectionMode(QTreeView::SingleSelection);
  resizeHeaders();
  setFocusProxy(ui->tree);

  updateModel(Sensor_v(), TimeSpan());
}

ErrorList::~ErrorList()
{
  METLIBS_LOG_SCOPE();
  delete mObsPgmRequest;
}

void ErrorList::onButtonSearch()
{
  if (not mDialog->exec())
    return;

  const hqc::int_v selectedStations = mDialog->getSelectedStations();
  const hqc::int_s stationIds(selectedStations.begin(), selectedStations.end());

  delete mObsPgmRequest;
  mObsPgmRequest = new ObsPgmRequest(stationIds);
  connect(mObsPgmRequest, SIGNAL(complete()), this, SLOT(onObsPgmsComplete()));
  mObsPgmRequest->post();
}
 
void ErrorList::onObsPgmsComplete()
{
  const TimeSpan timeLimits = mDialog->getTimeSpan();
  const hqc::int_v selectedStations = mDialog->getSelectedStations();
  const hqc::int_v selectedParameters = mDialog->getSelectedParameters();

  Sensor_v sensors;

  BOOST_FOREACH(int stationId, selectedStations) {
    BOOST_FOREACH(int paramId, selectedParameters) {
      const hqc::kvObsPgm_v& opl = mObsPgmRequest->get(stationId);
      Sensor sensor(stationId, paramId, 0, 0, 0);
      hqc::int_s typeIdsShown;
      BOOST_FOREACH(const kvalobs::kvObsPgm& op, opl) {
        const TimeSpan op_time(op.fromtime(), op.totime());
        if (timeLimits.intersection(op_time).undef())
          continue;
        const int p = op.paramID(), t = op.typeID();
        if (p == paramId) {
          sensor.typeId = t;
        } else if (Helpers::aggregatedParameter(p, paramId)) {
          sensor.typeId = -t;
        } else {
          continue;
        }
        if (typeIdsShown.find(sensor.typeId) == typeIdsShown.end()) {
          sensors.push_back(sensor);
          typeIdsShown.insert(sensor.typeId);
        }
      }
    }
  }

  delete mObsPgmRequest;
  mObsPgmRequest = 0;

  setSensorsAndTimes(sensors, timeLimits);
}

void ErrorList::setSensorsAndTimes(const Sensor_v& sensors, const TimeSpan& time)
{
  METLIBS_LOG_SCOPE();
  updateModel(sensors, time);
}

void ErrorList::updateModel(const Sensor_v& sensors, const TimeSpan& time)
{
  mNavigate.invalidate();
  NavigateHelper::Blocker block(mNavigate);

  mItemModel = std::auto_ptr<ErrorListModel>(new ErrorListModel(mDA, mMA));
  connect(mItemModel.get(), SIGNAL(beginDataChange()),
      this, SLOT(onBeginDataChange()));
  connect(mItemModel.get(), SIGNAL(endDataChange()),
      this, SLOT(onEndDataChange()));
  connect(mItemModel.get(), SIGNAL(fetchingData(bool)), ui->busyLabel, SLOT(setBusy(bool)));

  ui->tree->setModel(mItemModel.get());
  mItemModel->search(sensors, time, mErrorsForSalen);

  connect(ui->tree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
  resizeHeaders();
}

void ErrorList::resizeHeaders()
{
  QHeaderView* header = ui->tree->header();
  header->setResizeMode(QHeaderView::Interactive);
  header->resizeSections(QHeaderView::ResizeToContents);
  header->resizeSection(ErrorListModel::COL_STATION_ID,   100);
  header->resizeSection(ErrorListModel::COL_STATION_NAME, 160);
  header->resizeSection(ErrorListModel::COL_STATION_WMO,   45);
  header->resizeSection(ErrorListModel::COL_OBS_TIME,     150);
  header->resizeSection(ErrorListModel::COL_OBS_PARAM,     60);
  header->resizeSection(ErrorListModel::COL_OBS_TYPEID,    40);
  header->resizeSection(ErrorListModel::COL_OBS_ORIG,      60);
  header->resizeSection(ErrorListModel::COL_OBS_CORR,      60);
  header->resizeSection(ErrorListModel::COL_OBS_MODEL,     60);
  header->resizeSection(ErrorListModel::COL_OBS_FLAGS,    120);
}

void ErrorList::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  METLIBS_LOG_SCOPE();
  showSameStation();
  signalStationSelected();
}

void ErrorList::showSameStation()
{
  METLIBS_LOG_SCOPE();

  if (mItemModel.get()) {
    ObsData_p obs = getSelectedObs();
    const int stationId = obs ? obs->sensorTime().sensor.stationId : -1;
    mItemModel->highlightStation(stationId);
  }
}

void ErrorList::signalStationSelected()
{
  METLIBS_LOG_SCOPE();
  if (ObsData_p obs = getSelectedObs()) {
    const SensorTime& st = obs->sensorTime();
    NavigateHelper::Blocker block(mNavigate);
    if (mNavigate.go(st)) {
      METLIBS_LOG_DEBUG(LOGVAL(st));
      signalNavigateTo(st);
      return;
    }
  }

  // reach here if no obs or invalid
  mNavigate.invalidate();
}

void ErrorList::onBeginDataChange()
{
  METLIBS_LOG_SCOPE();
  mNavigate.block();

  if (QItemSelectionModel* selection = ui->tree->selectionModel())
    selection->clear();
}

void ErrorList::onEndDataChange()
{
  METLIBS_LOG_SCOPE();
  mNavigate.unblock();
}

void ErrorList::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_SCOPE(LOGVAL(st) << LOGVAL(st.valid()));
  if (not mNavigate.go(st))
    return;

  const QModelIndex idx = mItemModel->findSensorTime(st);
  if (not idx.isValid())
    return;

  NavigateHelper::Blocker block(mNavigate);
  // scrollTo must come before select, otherwise scrolling will not happen
  ui->tree->scrollTo(idx);
  if (QItemSelectionModel* selection = ui->tree->selectionModel())
    selection->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

ObsData_p ErrorList::getSelectedObs() const
{
  if (QItemSelectionModel* selection = ui->tree->selectionModel()) {
    const QModelIndexList selected = selection->selectedRows();
    if (selected.size() == 1)
      return mItemModel->findObs(selected.front());
  }
  return ObsData_p();
}

void ErrorList::onButtonExpand()
{
  ui->tree->expandAll();
}

void ErrorList::onButtonForget()
{
  // FIXME not implemented yet
}

void ErrorList::saveSettings(QSettings& settings)
{
  mDialog->saveSettings(settings);
}

void ErrorList::restoreSettings(QSettings& settings)
{
  mDialog->restoreSettings(settings);
}
