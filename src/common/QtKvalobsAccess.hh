
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

protected:
  virtual void newStationWithData(int stationId);

private Q_SLOTS:
  void onKvData(kvservice::KvObsDataListPtr data);
  void doReSubscribe();

private:
  void reSubscribe();

private:
  std::string mKvServiceSubscriberID;

  QTimer* mResubscribeTimer;
};

#endif // COMMON_QTKVALOBSACCESS_HH
