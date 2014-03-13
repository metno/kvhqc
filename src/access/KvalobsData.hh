
#ifndef ACCESS_KVALOBSDATA_HH
#define ACCESS_KVALOBSDATA_HH 1

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

  virtual timeutil::ptime tbtime() const
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
  kvalobs::kvData mKvData;
  bool mCreated;
};

HQC_TYPEDEF_P(KvalobsData);

#endif // ACCESS_KVALOBSDATA_HH
