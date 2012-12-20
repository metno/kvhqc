
#ifndef KvalobsData_hh
#define KvalobsData_hh 1

#include "ObsData.hh"
#include <kvalobs/kvData.h>

class KvalobsData : public ObsData {
public:
    KvalobsData(const kvalobs::kvData& d);
    virtual ~KvalobsData();

    virtual SensorTime sensorTime() const;

    virtual float original() const
        { return mKvData.original(); }

    virtual float corrected() const
        { return mKvData.corrected(); }

    virtual kvalobs::kvControlInfo controlinfo() const
        { return mKvData.controlinfo(); }

    const kvalobs::kvData& data() const
        { return mKvData; }

    kvalobs::kvData& data()
        { return mKvData; }

private:
    kvalobs::kvData mKvData;
};
typedef boost::shared_ptr<KvalobsData> KvalobsDataPtr;

#endif // KvalobsData_hh
