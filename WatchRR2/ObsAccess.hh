
#ifndef ObsAccess_hh
#define ObsAccess_hh 1

#include "ObsSubscription.hh"
#include "ObsUpdate.hh"
#include <boost/signals.hpp>

class ObsAccess : public boost::enable_shared_from_this<ObsAccess>, private boost::noncopyable {
public:
    virtual ~ObsAccess();
    virtual ObsDataPtr find(const SensorTime& st) = 0;
    virtual ObsDataPtr create(const SensorTime& st) = 0;
    virtual bool update(const std::vector<ObsUpdate>& updates) = 0;

    virtual void addSubscription(const ObsSubscription& s) = 0;
    virtual void removeSubscription(const ObsSubscription& s) = 0;

public:
    enum ObsDataChange { MODIFIED, CREATED, DESTROYED };
    boost::signal2<void, ObsDataChange, ObsDataPtr> obsDataChanged;
};
typedef boost::shared_ptr<ObsAccess> ObsAccessPtr;

#endif // ObsAccess_hh
