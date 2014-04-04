
#ifndef ACCESS_KVALOBSDATA_HH
#define ACCESS_KVALOBSDATA_HH 1

#include "ObsData.hh"
#include <kvalobs/kvData.h>

class KvalobsData : public ObsData {
public:
  KvalobsData(const kvalobs::kvData& d, bool created);
  virtual ~KvalobsData();

  virtual const SensorTime& sensorTime() const
    { return mSensorTime; }

  virtual float original() const
    { return mKvData.original(); }

  virtual float corrected() const
    { return mKvData.corrected(); }

  virtual const kvalobs::kvControlInfo& controlinfo() const
    { return mKvData.controlinfo(); }

  virtual const std::string& cfailed() const
    { return mKvData.cfailed(); }

  virtual const timeutil::ptime& tbtime() const
    { return mKvData.tbtime(); }

  const kvalobs::kvData& data() const
    { return mKvData; }

  kvalobs::kvData& data()
    { return mKvData; }

  bool isCreated() const
    { return mCreated; }

  void setCreated(bool c)
    { mCreated = c; }

private:
  SensorTime mSensorTime;
  kvalobs::kvData mKvData;
  bool mCreated;
};

HQC_TYPEDEF_P(KvalobsData);

#endif // ACCESS_KVALOBSDATA_HH
