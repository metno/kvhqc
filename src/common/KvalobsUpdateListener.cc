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

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "KvalobsUpdateListener.hh"

#include "QtKvService.hh"

#include <kvalobs/kvData.h>

#include <QTimer>

#define MILOGGER_CATEGORY "kvhqc.KvalobsUpdateListener"
#include "util/HqcLogging.hh"

KvalobsUpdateListener::KvalobsUpdateListener()
  : mResubscribeTimer(new QTimer(this))
  , qtkvs(qtKvService())
{
  METLIBS_LOG_SCOPE();
  mResubscribeTimer->setSingleShot(true);
  connect(mResubscribeTimer, SIGNAL(timeout()), this, SLOT(doReSubscribe()));
  setUpdateListener(this);
}

KvalobsUpdateListener::~KvalobsUpdateListener()
{
  METLIBS_LOG_SCOPE();
  if (not mKvServiceSubscriberID.empty())
    qtKvService()->unsubscribe(mKvServiceSubscriberID);
  if (not mSubscribedStations.empty())
    HQC_LOG_WARN("station list not empty");
  setUpdateListener(0);
}

void KvalobsUpdateListener::onKvData(kvservice::KvObsDataListPtr dl)
{
  METLIBS_LOG_SCOPE();
  for (kvservice::KvObsData& od : *dl) {
    for (const kvalobs::kvData& kvd : od.dataList()) {
      METLIBS_LOG_DEBUG("updated: " << kvd);
      Q_EMIT update(kvd);
    }

    const hqc::kvData_v data(od.dataList().begin(), od.dataList().end());
    Q_EMIT updated(data);
  }
}

void KvalobsUpdateListener::addStation(int stationId)
{
  METLIBS_LOG_SCOPE(LOGVAL(stationId));
  if (mSubscribedStations.increase(stationId))
    reSubscribe();
}

void KvalobsUpdateListener::removeStation(int stationId)
{
  METLIBS_LOG_SCOPE(LOGVAL(stationId));
  if (mSubscribedStations.decrease(stationId))
    reSubscribe();
}

void KvalobsUpdateListener::reSubscribe()
{
  METLIBS_LOG_SCOPE();
  mResubscribeTimer->start(100 /*ms*/);
}

void KvalobsUpdateListener::doReSubscribe()
{
  METLIBS_LOG_TIME();
  if (not mKvServiceSubscriberID.empty()) {
    qtKvService()->unsubscribe(mKvServiceSubscriberID);
    mKvServiceSubscriberID = "";
  }

  if (not mSubscribedStations.empty()) {
    kvservice::KvDataSubscribeInfoHelper dataSubscription;
    for (station_count_t::const_iterator it=mSubscribedStations.begin(); it != mSubscribedStations.end(); ++it)
      dataSubscription.addStationId(it->first);

    mKvServiceSubscriberID = qtKvService()
        ->subscribeData(dataSubscription, this, SLOT(onKvData(kvservice::KvObsDataListPtr)));
    METLIBS_LOG_DEBUG("new id=" << mKvServiceSubscriberID);
  }
}
