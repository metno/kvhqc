
#ifndef QtKvalobsAccess_hh
#define QtKvalobsAccess_hh 1

#include "KvalobsAccess.hh"

#include <map>

class QTimer;

class QtKvalobsAccess : public QObject, public KvalobsAccess
{ Q_OBJECT;
public:
    QtKvalobsAccess();
    ~QtKvalobsAccess();

    virtual void addSubscription(const ObsSubscription& s);
    virtual void removeSubscription(const ObsSubscription& s);

private Q_SLOTS:
    void onKvData(kvservice::KvObsDataListPtr data);
    void doReSubscribe();

private:
    void reSubscribe();

private:
  std::string mKvServiceSubscriberID;

  typedef std::map<int, int> SubscribedStations_t;
  SubscribedStations_t mSubscribedStations;

  QTimer* mResubscribeTimer;
};

#endif // KvalobsAccess_hh
