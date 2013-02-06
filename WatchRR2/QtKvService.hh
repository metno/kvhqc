
#ifndef QtCorbaKvApp_hh
#define QtKvService_hh

#include <kvcpp/corba/CorbaKvApp.h>
#include <kvcpp/kvDataSubscribeInfoHelper.h>
#include <QtCore/QThread>

class QtKvService : public QThread
{ Q_OBJECT;
public:
    QtKvService();
    virtual ~QtKvService();

    typedef kvservice::corba::CorbaKvApp::SubscriberID SubscriberID;

    SubscriberID subscribeDataNotify(const kvservice::KvDataSubscribeInfoHelper &info,
                                     const QObject *receiver=0, const char *member=0);

    SubscriberID subscribeData(const kvservice::KvDataSubscribeInfoHelper &info,
                               const QObject *receiver=0, const char* member=0);

    SubscriberID subscribeKvHint(const QObject *receiver=0, const char *member=0);

    void unsubscribe(const SubscriberID& subscriberId);
    
Q_SIGNALS:
    void kvDataNotify(kvservice::KvWhatListPtr what);
    void kvData(kvservice::KvObsDataListPtr data);
    void kvHint(bool comingUp);

    void internalKvDataNotify(kvservice::KvWhatListPtr);
    void internalKvData(kvservice::KvObsDataListPtr);
    void internalKvHint(bool);

private Q_SLOTS:
    void internalSendKvDataNotify(kvservice::KvWhatListPtr);
    void internalSendKvData(kvservice::KvObsDataListPtr);
    void internalSendKvHint(bool);

protected:
    virtual void run();

private:
    void stop();
    SubscriberID connectSubscriptionSignal(const SubscriberID& subscriberId, const char* emitter,
                                           const QObject *receiver=0, const char* member=0);

private:
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
