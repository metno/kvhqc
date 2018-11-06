/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

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


#include "QtKvService.hh"

#include <kvcpp/kvevents.h>

#include <QCoreApplication>
#include <QMetaType>

#define MILOGGER_CATEGORY "kvhqc.QtKvService"
#include "util/HqcLogging.hh"

namespace /* anonymous */ {

QtKvService* qkvs = 0;

} // namespace anonymous

QtKvService::QtKvService(std::shared_ptr<kvservice::KvApp> app)
    : mApp(app)
    , mStop(false)
    , mStopped(false)
{
  assert(not qkvs);
  qkvs = this;

  if (not QMetaType::type("kvservice::KvObsDataListPtr"))
    qRegisterMetaType<kvservice::KvObsDataListPtr>("kvservice::KvObsDataListPtr");
  if (not QMetaType::type("kvservice::KvWhatListPtr"))
    qRegisterMetaType<kvservice::KvWhatListPtr>("kvservice::KvWhatListPtr");

  start();
}

QtKvService::~QtKvService()
{
  METLIBS_LOG_SCOPE();

  if (not mStopped) {
    HQC_LOG_WARN("kvService CORBA connector not yet stopped");
    stop();
  }

  if (not mSubscriptions.empty())
    HQC_LOG_WARN("kvService CORBA connector destructor: "
        << mSubscriptions.size() << " subscribers remaining");

  for(Subscriptions_t::const_iterator it = mSubscriptions.begin(); it != mSubscriptions.end(); ++it) {
    if (mApp)
      mApp->unsubscribe(it->first);
    else
      HQC_LOG_WARN("no app, cannot unsubscribe '" << it->first << "'");
#if 0 // Qt should disconnect (or already have disconnected) these
    const Subscriber& s = it->second;
    disconnect(this, s.emitted, s.receiver, s.member);
#endif
  }

  qkvs = 0;
}

QtKvService::SubscriberID QtKvService::connectSubscriptionSignal(const SubscriberID& subscriberId,
    const char* emitted, const QObject *receiver, const char* member)
{
  if ((not subscriberId.empty()) and receiver and member) {
    if (not connect(this, emitted, receiver, member)) {
      HQC_LOG_ERROR("failed to connect signal, unsubscribing again");
      mApp->unsubscribe(subscriberId);
      return "";
    }
  }
  mSubscriptions.insert(Subscriptions_t::value_type(subscriberId, Subscriber(emitted, receiver, member)));
  return subscriberId;
}

QtKvService::SubscriberID QtKvService::subscribeDataNotify(const kvservice::KvDataSubscribeInfoHelper &info,
    const QObject *receiver, const char *member)
{
  METLIBS_LOG_SCOPE();
  return connectSubscriptionSignal(mApp->subscribeDataNotify(info, mSignalQueue), SIGNAL(kvData(kvservice::KvWhatListPtr)), receiver, member);
}

QtKvService::SubscriberID QtKvService::subscribeData(const kvservice::KvDataSubscribeInfoHelper &info,
    const QObject *receiver, const char* member)
{
  METLIBS_LOG_SCOPE();
  return connectSubscriptionSignal(mApp->subscribeData(info, mSignalQueue), SIGNAL(kvData(kvservice::KvObsDataListPtr)), receiver, member);
}

QtKvService::SubscriberID QtKvService::subscribeKvHint(const QObject *receiver, const char *member)
{
  METLIBS_LOG_SCOPE();
  return connectSubscriptionSignal(mApp->subscribeKvHint(mSignalQueue), SIGNAL(kvHint(bool)), receiver, member);
}

void QtKvService::unsubscribe(const SubscriberID& subscriberId)
{
  METLIBS_LOG_SCOPE();
  Subscriptions_t::iterator it = mSubscriptions.find(subscriberId);
  if (it != mSubscriptions.end()) {
    if (mApp)
      mApp->unsubscribe(subscriberId);
    else
      HQC_LOG_WARN("no app, cannot unsubscribe '" << subscriberId << "'");

    const Subscriber& s = it->second;
    disconnect(this, s.emitted, s.receiver, s.member);
    mSubscriptions.erase(it);
  }
}

void QtKvService::run()
{
  METLIBS_LOG_SCOPE();
  using namespace kvservice;
  while (not mStop and mApp and not mApp->shutdown()) {
    const std::auto_ptr<dnmi::thread::CommandBase> com(mSignalQueue.get(/*timeout=*/ 2 /*sec*/));
    if (not com.get())
      continue;

    if (DataEvent *dataEvent = dynamic_cast<DataEvent*>(com.get())) {
      METLIBS_LOG_DEBUG("got DataEvent");
      Q_EMIT kvData(dataEvent->data());
    } else if (DataNotifyEvent *dataNotifyEvent = dynamic_cast<DataNotifyEvent*>(com.get())) {
      METLIBS_LOG_DEBUG("got DataNotifyEvent");
      Q_EMIT kvDataNotify(dataNotifyEvent->what());
    } else if (HintEvent *hintEvent = dynamic_cast<HintEvent*>(com.get())) {
      METLIBS_LOG_DEBUG("got HintEvent");
      Q_EMIT kvHint(hintEvent->upEvent());
    }
  }
  Q_EMIT shutdown();
}

void QtKvService::stop()
{
  METLIBS_LOG_SCOPE();
  mStop = true;
  mSignalQueue.signal();
  METLIBS_LOG_INFO("Waiting for kvService CORBA connector ...");

  wait();
  METLIBS_LOG_INFO("kvService CORBA connector stopped");
  mStopped = true;
}

QtKvService* qtKvService()
{
  return qkvs;
}
