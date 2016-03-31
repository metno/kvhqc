
#ifndef COMMON_KVALOBSUPDATELISTENER_HH
#define COMMON_KVALOBSUPDATELISTENER_HH 1

#include "common/AbstractUpdateListener.hh"
#include <kvcpp/kvservicetypes.h>
#include <map>

class QTimer;

/*! Access to kvalobs data, with update listener.
 * Uses QtKvService to listen for updates in kvalobs database.
 */
class KvalobsUpdateListener : public AbstractUpdateListener
{ Q_OBJECT;
public:
  KvalobsUpdateListener();
  virtual ~KvalobsUpdateListener();

  virtual void addStation(int stationId);
  virtual void removeStation(int stationId);

private Q_SLOTS:
  void onKvData(kvservice::KvObsDataListPtr data);
  void doReSubscribe();

private:
  void reSubscribe();

private:
  std::string mKvServiceSubscriberID;
  QTimer* mResubscribeTimer;

  typedef std::map<int, int> station_count_t;
  station_count_t mSubscribedStations;
};

#endif // COMMON_KVALOBSUPDATELISTENER_HH
