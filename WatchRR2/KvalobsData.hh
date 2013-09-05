
#ifndef KvalobsData_hh
#define KvalobsData_hh 1

#include "ObsData.hh"
#include <kvalobs/kvData.h>

class KvalobsData : public ObsData {
public:
  KvalobsData(const kvalobs::kvData& d, bool created);
  virtual ~KvalobsData();

  virtual SensorTime sensorTime() const;

  virtual float original() const
    { return mKvData.original(); }

  virtual float corrected() const
    { return mKvData.corrected(); }

  virtual kvalobs::kvControlInfo controlinfo() const
    { return mKvData.controlinfo(); }

  virtual std::string cfailed() const
    { return mKvData.cfailed(); }

  const kvalobs::kvData& data() const
    { return mKvData; }

  kvalobs::kvData& data()
    { return mKvData; }

  bool isCreated() const
    { return mCreated; }

  bool setCreated(bool c)
    { mCreated = c; }

private:
  kvalobs::kvData mKvData;
  bool mCreated;
};
typedef boost::shared_ptr<KvalobsData> KvalobsDataPtr;

#endif // KvalobsData_hh
