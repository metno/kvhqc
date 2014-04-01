
#ifndef EditAccess_hh
#define EditAccess_hh 1

#include "EditDataEditor.hh"
#include "ObsAccess.hh"

/*! Access to editable data. Supports undo and redo.
 */
class EditAccess : public ObsAccess {
public:
  /*! Edit data from \c backend */
  EditAccess(ObsAccess_p backend);
  ~EditAccess();

  virtual void postRequest(ObsRequest_p request);
  virtual void dropRequest(ObsRequest_p request);

  virtual ObsUpdate_p createUpdate(ObsData_p& data);
  virtual ObsUpdate_p createUpdate(const SensorTime& sensorTime);
  virtual bool storeUpdates(const ObsUpdate_pv& updates);

  virtual bool storeToBackend();

  void newVersion();
  void undoVersion();
  void redoVersion();
  bool canUndo() const
    { return mCurrentVersion > 0; }
  bool canRedo() const
    { return mCurrentVersion < highestVersion(); }
  int highestVersion() const
    { return mVersionTimestamps.size() - 1; }
  int currentVersion() const
    { return mCurrentVersion; }
  int countU() const
    { return mUpdated; }

  const timeutil::ptime& versionTimestamp(int version) const
    { return mVersionTimestamps[version]; }

  typedef std::vector<EditDataPtr> ChangedData_t;
  ChangedData_t versionChanges(int version) const;

  void reset();

Q_SIGNALS:
  void currentVersionChanged(int current, int highest);

private:
  void sendObsDataChanged(ObsDataChange what, ObsDataPtr obs, int dUpdated, int dTasks);
  void onBackendDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);
  void updateToCurrentVersion(bool drop);
  void addEditTimes(TimeSet& times, const std::vector<Sensor>& sensors, const TimeRange& limits);

private:
  ObsAccess_p mBackend;
  typedef std::map<SensorTime, EditDataPtr, lt_SensorTime> Data_t;
  Data_t mData;

  typedef std::vector<timeutil::ptime> VersionTimestamps_v;
  VersionTimestamps_v mVersionTimestamps;

  int mCurrentVersion;
};
typedef boost::shared_ptr<EditAccess> EditAccessPtr;

#endif // EditAccess_hh
