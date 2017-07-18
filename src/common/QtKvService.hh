
#ifndef QtCorbaKvApp_hh
#define QtKvService_hh

#include <kvcpp/KvApp.h>
#include <kvcpp/kvDataSubscribeInfoHelper.h>
#include <QThread>

class QtKvService : public QThread
{ Q_OBJECT;
public:
  QtKvService(std::shared_ptr<kvservice::KvApp> app);
  virtual ~QtKvService();

  typedef kvservice::KvApp::SubscriberID SubscriberID;

  SubscriberID subscribeDataNotify(const kvservice::KvDataSubscribeInfoHelper &info,
      const QObject *receiver=0, const char *member=0);

  SubscriberID subscribeData(const kvservice::KvDataSubscribeInfoHelper &info,
      const QObject *receiver=0, const char* member=0);

  SubscriberID subscribeKvHint(const QObject *receiver=0, const char *member=0);

  void unsubscribe(const SubscriberID& subscriberId);
  void stop();

Q_SIGNALS:
  void kvDataNotify(kvservice::KvWhatListPtr what);
  void kvData(kvservice::KvObsDataListPtr data);
  void kvHint(bool comingUp);
  void shutdown();

protected:
  virtual void run();

private:
  SubscriberID connectSubscriptionSignal(const SubscriberID& subscriberId, const char* emitter,
      const QObject *receiver=0, const char* member=0);

private:
  std::shared_ptr<kvservice::KvApp> mApp;
  dnmi::thread::CommandQue mSignalQueue;
  bool mStop;
  bool mStopped;

  struct Subscriber {
    const char* emitted;
    const QObject* receiver;
    const char* member;
    Subscriber(const char* e, const QObject* r, const char* m) : emitted(e), receiver(r), member(m) { }
  };
  typedef std::map<SubscriberID, Subscriber> Subscriptions_t;
  Subscriptions_t mSubscriptions;
};

QtKvService* qtKvService();

#endif // QtKvService_hh
