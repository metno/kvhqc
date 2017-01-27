
#ifndef ACCESS_KVALOBSDATA_HH
#define ACCESS_KVALOBSDATA_HH 1

#include "ObsData.hh"
#include <kvalobs/kvData.h>

class KvalobsData : public ObsData {
public:
  KvalobsData(const kvalobs::kvData& d, bool created);
  virtual ~KvalobsData();

  const SensorTime& sensorTime() const Q_DECL_OVERRIDE
    { return mSensorTime; }

  float original() const Q_DECL_OVERRIDE
    { return mKvData.original(); }

  float corrected() const Q_DECL_OVERRIDE
    { return mKvData.corrected(); }

  const kvalobs::kvControlInfo& controlinfo() const Q_DECL_OVERRIDE
    { return mKvData.controlinfo(); }

  const std::string& cfailed() const Q_DECL_OVERRIDE
    { return mKvData.cfailed(); }

  const timeutil::ptime& tbtime() const Q_DECL_OVERRIDE
    { return mKvData.tbtime(); }

  const kvalobs::kvData& data() const
    { return mKvData; }

  kvalobs::kvData& data()
    { return mKvData; }

  bool isCreated() const
    { return mCreated; }

  void setCreated(bool c)
    { mCreated = c; }

  bool isModified() const Q_DECL_OVERRIDE
    { return mModified; }

  void setModified(bool m)
    { mModified = m; }

private:
  SensorTime mSensorTime;
  kvalobs::kvData mKvData;
  bool mCreated;
  bool mModified;
};

HQC_TYPEDEF_P(KvalobsData);
HQC_TYPEDEF_PV(KvalobsData);

#endif // ACCESS_KVALOBSDATA_HH
