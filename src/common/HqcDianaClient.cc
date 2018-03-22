/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2014-2018 met.no

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

#include "HqcDianaClient.hh"

#include "../hqc/HqcAppWindow.hh"
#include "../hqc/ViewChanges.hh"

#include "common/DianaHelper.hh"

#include "util/Helpers.hh"
#include "util/hqc_paths.hh"

#include <QAction>
#include <QShortcut>
#include <QStatusBar>

#define MILOGGER_CATEGORY "kvhqc.HqcDianaClient"
#include "common/ObsLogging.hh"

HqcDianaClient::HqcDianaClient(DianaHelper* dh, QObject* parent)
    : DianaClient(parent)
    , mDianaHelper(dh)
{
  mActionForToolButton = new QAction(this);
  mActionForToolButton->setText("Co4");
  mActionForToolButton->setCheckable(true);
  mActionForToolButton->setChecked(true);
  connect(mActionForToolButton, &QAction::toggled, this, &HqcDianaClient::makeActive);

  makeActive(true);
}

HqcDianaClient::~HqcDianaClient()
{
  METLIBS_LOG_SCOPE();
  makeActive(false);
}

void HqcDianaClient::makeActive(bool active)
{
  METLIBS_LOG_SCOPE();
  mDianaHelper->setClientActive(this, active);
}

void HqcDianaClient::setActive(bool active)
{
  METLIBS_LOG_SCOPE();
  if (active)
    mActionForToolButton->setToolTip(tr("Communication with diana is enabled"));
  else
    mActionForToolButton->setToolTip(tr("Communication with diana is disabled"));
  mActionForToolButton->setChecked(active);
}

void HqcDianaClient::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_SCOPE(LOGVAL(st_) << LOGVAL(st));
  if (!st.valid() || eq_SensorTime()(st, st_))
    return;

  bool sendtime = st_.time != st.time;
  if (doNavigateTo(st)) {
    sendtime = true;
    Q_EMIT clientTimesChanged(getClientTimes());
  }
  if (sendtime)
    Q_EMIT clientTimeSelected(getClientTime());
}

bool HqcDianaClient::doNavigateTo(const SensorTime& st)
{
  // if paramid changes, we need to build a new time list
  bool timeListChanged = !eq_Sensor()(st.sensor, st_.sensor);

  st_ = st;
  if (timeListChanged || !ts_.closed() || !ts_.contains(st_.time)) {
    ts_ = ViewChanges::defaultTimeLimits(st_);
    timeListChanged = true;
    times_ = ts_.expand(3600);
  }
  return timeListChanged;
}

void HqcDianaClient::remoteTimeSelected(const timeutil::ptime& time)
{
  METLIBS_LOG_SCOPE(LOGVAL(st_) << LOGVAL(time));
  st_.time = time;
  Q_EMIT signalNavigateTo(st_);
}

timeutil::ptime HqcDianaClient::getClientTime()
{
  return st_.time;
}

std::vector<timeutil::ptime> HqcDianaClient::getClientTimes()
{
  return times_;
}
