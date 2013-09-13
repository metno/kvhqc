
#include "QtKvService.hh"

#include <kvcpp/kvevents.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaType>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.QtKvService"
#include "util/HqcLogging.hh"

namespace /* anonymous */ {

inline kvservice::corba::CorbaKvApp* app() {
    return static_cast<kvservice::corba::CorbaKvApp*>(kvservice::KvApp::kvApp);
}

QtKvService* qkvs = 0;

} // namespace anonymous

QtKvService::QtKvService()
    : mStop(false)
    , mStopped(false)
{
    assert(not qkvs);
    qkvs = this;

    qRegisterMetaType<kvservice::KvObsDataListPtr>("kvservice::KvObsDataListPtr");
    qRegisterMetaType<kvservice::KvWhatListPtr>("kvservice::KvWhatListPtr");

    // the signals "internalKvXyz" are emitted from the CORBA POA's thread
    // the slots "internalSendKvXyz" are called from the main/GUI thread
    connect(this, SIGNAL(internalKvData(kvservice::KvObsDataListPtr)),
            this, SLOT(internalSendKvData(kvservice::KvObsDataListPtr)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(internalKvDataNotify(kvservice::KvWhatListPtr)),
            this, SLOT(internalSendKvDataNotify(kvservice::KvWhatListPtr)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(internalKvHint(bool)),
            this, SLOT(internalSendKvHint(bool)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(internalShutdown()),
            this, SLOT(internalSendShutdown()),
            Qt::QueuedConnection);

    start();
}

QtKvService::~QtKvService()
{
  METLIBS_LOG_SCOPE();

  if (not mStopped)
    stop();

  BOOST_FOREACH(Subscriptions_t::value_type& sub, mSubscriptions) {
    if (app())
      app()->unsubscribe(sub.first);
    else {
      using ::milogger::detail::Priority;
      HQC_LOG_WARN("no app, cannot unsubscribe '" << sub.first << "'");
    }
    const Subscriber& s = sub.second;
    disconnect(this, s.emitted, s.receiver, s.member);
  }

  qkvs = 0;
}

QtKvService::SubscriberID QtKvService::connectSubscriptionSignal(const SubscriberID& subscriberId,
                                                                 const char* emitted, const QObject *receiver, const char* member)
{
    if ((not subscriberId.empty()) and receiver and member) {
        if (not connect(this, emitted, receiver, member)) {
            using ::milogger::detail::Priority;
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
    else {
      using ::milogger::detail::Priority;
      HQC_LOG_WARN("no app, cannot unsubscribe '" << subscriberId << "'");
    }
      
    const Subscriber& s = it->second;
    disconnect(this, s.emitted, s.receiver, s.member);
    mSubscriptions.erase(it);
  }
}

void QtKvService::internalSendKvDataNotify(kvservice::KvWhatListPtr data)
{
    // this function is run in the GUI thread
    /*emit*/ kvDataNotify(data);
}

void QtKvService::internalSendKvData(kvservice::KvObsDataListPtr data)
{
    // this function is run in the GUI thread
    /*emit*/ kvData(data);
}

void QtKvService::internalSendKvHint(bool c)
{
    // this function is run in the GUI thread
    /*emit*/ kvHint(c);
}

void QtKvService::internalSendShutdown()
{
    // this function is run in the GUI thread
    /*emit*/ shutdown();
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
      /*emit*/ internalKvData(dataEvent->data());
    } else if (DataNotifyEvent *dataNotifyEvent = dynamic_cast<DataNotifyEvent*>(com.get())) {
      /*emit*/ internalKvDataNotify(dataNotifyEvent->what());
    } else if (HintEvent *hintEvent = dynamic_cast<HintEvent*>(com.get())) {
      /*emit*/ internalKvHint(hintEvent->upEvent());
    }
  }
  /*emit*/ internalShutdown();
}

void QtKvService::stop()
{
  METLIBS_LOG_SCOPE();
  mStop = true;
  mSignalQueue.signal();
  using ::milogger::detail::Priority;
  METLIBS_LOG_INFO("Waiting for kvService CORBA connector ...");

  wait();
  METLIBS_LOG_INFO("kvService CORBA connector stopped");
  mStopped = true;
}

QtKvService* qtKvService()
{
  return qkvs;
}
