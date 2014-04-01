
#ifndef EditData_hh
#define EditData_hh 1

#include "Functors.hh"
#include "ObsData.hh"
#include "util/VersionedValue.hh"

class EditData;
typedef boost::shared_ptr<EditData> EditDataPtr;

class EditData : public ObsData {
public:
  EditData(ObsData_p data);

  virtual const SensorTime& sensorTime() const
    { return mData->sensorTime(); }

  virtual float original() const
    { return mOriginal; }

  virtual float corrected() const
    { return mCorrected.value(); }

  virtual const kvalobs::kvControlInfo& controlinfo() const
    { return mControlinfo.value(); }

  virtual const std::string& cfailed() const
    { return mData->cfailed(); }

  virtual const timeutil::ptime& tbtime() const
    { return mData->tbtime(); }

  float oldCorrected() const
    { return mCorrected.original();; }

  const kvalobs::kvControlInfo& oldControlinfo() const
    { return mControlinfo.original(); }

  bool modified() const
    { return (modifiedCorrected() or modifiedControlinfo()); }

  bool modifiedCorrected() const
    { return mCreated or mCorrected.modified(); }

  bool modifiedControlinfo() const
    { return mCreated or mControlinfo.modified(); }

  bool hasVersion(int version)
    { return mCorrected.hasVersion(version) or mControlinfo.hasVersion(version) or mTasks.hasVersion(version); }

  float corrected(int version) const
    { return mCorrected.value(version); }

  const kvalobs::kvControlInfo& controlinfo(int version) const
    { return mControlinfo.value(version); }

  bool created() const
    { return mCreated; }

private:
  bool updateFromBackend();
  void reset();

private:
  ObsData_p mData;
  bool mCreated;
  float mOriginal;

  typedef VersionedValue<float, Helpers::float_eq> Corrected_t;
  typedef VersionedValue<kvalobs::kvControlInfo>   Controlinfo_t;
  typedef VersionedValue<int>                      Tasks_t;

  Corrected_t   mCorrected;
  Controlinfo_t mControlinfo;
  Tasks_t       mTasks;

  friend class EditAccess;
  friend class EditDataEditor;
};

HQC_TYPEDEF_P(EditData);

#endif // EditData_hh
