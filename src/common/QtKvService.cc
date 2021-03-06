
#include "QtKvService.hh"

#include <kvcpp/kvevents.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaType>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.QtKvService"
#include "util/HqcLogging.hh"

namespace /* anonymous */ {

inline kvservice::KvApp* app() {
  return static_cast<kvservice::KvApp*>(kvservice::KvApp::kvApp);
}

QtKvService* qkvs = 0;

} // namespace anonymous

QtKvService::QtKvService()
  : mStop(false)
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

  BOOST_FOREACH(Subscriptions_t::value_type& sub, mSubscriptions) {
    if (app())
      app()->unsubscribe(sub.first);
    else
      HQC_LOG_WARN("no app, cannot unsubscribe '" << sub.first << "'");
#if 0 // Qt should disconnect (or already have disconnected) these
    const Subscriber& s = sub.second;
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
      app()->unsubscribe(subscriberId);
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
  return connectSubscriptionSignal(app()->subscribeDataNotify(info, mSignalQueue),
      SIGNAL(kvData(kvservice::KvWhatListPtr)),
      receiver, member);
}

QtKvService::SubscriberID QtKvService::subscribeData(const kvservice::KvDataSubscribeInfoHelper &info,
    const QObject *receiver, const char* member)
{
  METLIBS_LOG_SCOPE();
  return connectSubscriptionSignal(app()->subscribeData(info, mSignalQueue),
      SIGNAL(kvData(kvservice::KvObsDataListPtr)),
      receiver, member);
}

QtKvService::SubscriberID QtKvService::subscribeKvHint(const QObject *receiver, const char *member)
{
  METLIBS_LOG_SCOPE();
  return connectSubscriptionSignal(app()->subscribeKvHint(mSignalQueue),
      SIGNAL(kvHint(bool)), receiver, member);
}

void QtKvService::unsubscribe(const SubscriberID& subscriberId)
{
  METLIBS_LOG_SCOPE();
  Subscriptions_t::iterator it = mSubscriptions.find(subscriberId);
  if (it != mSubscriptions.end()) {
    if (app())
      app()->unsubscribe(subscriberId);
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
  while (not mStop and app() and not app()->shutdown()) {
    const std::auto_ptr<dnmi::thread::CommandBase> com(mSignalQueue.get(/*timeout=*/ 2 /*sec*/));
    if (not com.get())
      continue;

    if (DataEvent *dataEvent = dynamic_cast<DataEvent*>(com.get())) {
      Q_EMIT kvData(dataEvent->data());
    } else if (DataNotifyEvent *dataNotifyEvent = dynamic_cast<DataNotifyEvent*>(com.get())) {
      Q_EMIT kvDataNotify(dataNotifyEvent->what());
    } else if (HintEvent *hintEvent = dynamic_cast<HintEvent*>(com.get())) {
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
