
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
  virtual void findRange(const std::vector<Sensor>& sensors, const TimeRange& limits);

private Q_SLOTS:
  void onKvData(kvservice::KvObsDataListPtr data);
  void doReSubscribe();

private:
  void reSubscribe();

private:
  std::string mKvServiceSubscriberID;
  QTimer* mResubscribeTimer;

  typedef std::set<int> stations_with_data_t;
  stations_with_data_t mStationsWithData;
};

#endif // COMMON_QTKVALOBSACCESS_HH
