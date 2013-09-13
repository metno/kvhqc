
#ifndef COMMON_KVBUFFEREDACCESS_HH
#define COMMON_KVBUFFEREDACCESS_HH 1

#include "ObsAccess.hh"

namespace kvalobs {
class kvData;
}
class KvalobsData;
typedef boost::shared_ptr<KvalobsData> KvalobsDataPtr;

/*! Buffered ObsAccess for KvalobsData.
 * Has no methods to actually fetch and store data from a kvalobs database.
 */
class KvBufferedAccess : public ObsAccess {
public:
  virtual TimeSet allTimes(const std::vector<Sensor>& sensors, const TimeRange& limits);
  virtual DataSet allData(const std::vector<Sensor>& sensors, const TimeRange& limits);
  using ObsAccess::allTimes;
  using ObsAccess::allData;

  virtual ObsDataPtr find(const SensorTime& st);
  virtual ObsDataPtr create(const SensorTime& st);

  virtual void addSubscription(const ObsSubscription& s);
  virtual void removeSubscription(const ObsSubscription& s);

protected:
  /*! Receive data. Adds to buffer only if subscribed.
   * \param update true if this is an external update
   */
  virtual void receive(const kvalobs::kvData& data, bool update);

  virtual bool drop(const SensorTime& st);

  bool updatesHaveTasks(const std::vector<ObsUpdate>& updates);

  virtual bool isSubscribed(const SensorTime& st);
    
private:
  typedef std::vector<ObsSubscription> Subscriptions_t;
  Subscriptions_t mSubscriptions;

  typedef std::map<int, TimeRange> SubscribedTimes_t;
  SubscribedTimes_t mSubscribedTimes;

  typedef std::map<SensorTime, KvalobsDataPtr, lt_SensorTime> Data_t;
  Data_t mData;
};
typedef boost::shared_ptr<KvBufferedAccess> KvBufferedAccessPtr;

#endif // COMMON_KVBUFFEREDACCESS_HH