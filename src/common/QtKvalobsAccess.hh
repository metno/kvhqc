
#ifndef COMMON_QTKVALOBSACCESS_HH
#define COMMON_QTKVALOBSACCESS_HH 1

#include "KvalobsAccess.hh"

#include <map>

class QTimer;

/*! Access to kvalobs data, with update listener.
 * Uses QtKvService to listen for updates in kvalobs database.
 */
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

#endif // COMMON_QTKVALOBSACCESS_HH
