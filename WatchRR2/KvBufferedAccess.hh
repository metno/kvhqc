
#ifndef KvBufferedAccess_hh
#define KvBufferedAccess_hh 1

#include "KvalobsData.hh"
#include "ObsAccess.hh"

class KvBufferedAccess : public ObsAccess {
public:
    virtual ObsDataPtr find(const SensorTime& st);
    virtual ObsDataPtr create(const SensorTime& st);
    virtual bool update(const std::vector<ObsUpdate>& updates);

protected:
    KvalobsDataPtr receive(const kvalobs::kvData& data);
    bool drop(const SensorTime& st);

protected:
    typedef std::map<SensorTime, KvalobsDataPtr, lt_SensorTime> Data_t;
    Data_t mData;
};
typedef boost::shared_ptr<KvBufferedAccess> KvBufferedAccessPtr;

#endif // KvBufferedAccess_hh
