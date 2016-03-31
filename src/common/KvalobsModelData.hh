
#ifndef KvalobsModelData_hh
#define KvalobsModelData_hh 1

#include "ModelData.hh"
#include <kvalobs/kvModelData.h>

class KvalobsModelData : public ModelData {
public:
    KvalobsModelData(const kvalobs::kvModelData& m);
    virtual ~KvalobsModelData();

    virtual SensorTime sensorTime() const;

    virtual float value() const
        { return mKvData.original(); }

    const kvalobs::kvModelData& data() const
        { return mKvData; }

    kvalobs::kvModelData& data()
        { return mKvData; }

private:
    kvalobs::kvModelData mKvData;
};
typedef std::shared_ptr<KvalobsModelData> KvalobsModelDataPtr;

#endif // KvalobsModelData_hh
