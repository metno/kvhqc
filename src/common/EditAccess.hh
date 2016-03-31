
#ifndef EditAccess_hh
#define EditAccess_hh 1

#include "EditDataEditor.hh"
#include "ObsAccess.hh"

/*! Access to editable data. Supports undo and redo.
 */
class EditAccess : public ObsAccess {
public:
  /*! Edit data from \c backend */
  EditAccess(ObsAccessPtr backend);
  virtual ~EditAccess();

  virtual TimeSet allTimes(const std::vector<Sensor>& sensors, const TimeRange& limits);
  virtual DataSet allData(const std::vector<Sensor>& sensors, const TimeRange& limits);
  using ObsAccess::allTimes;
  using ObsAccess::allData;

  virtual ObsDataPtr find(const SensorTime& st);

  /*! Same as find, only difference is return type. */
  EditDataPtr findE(const SensorTime& st)
    { return std::static_pointer_cast<EditData>(find(st)); }

  virtual ObsDataPtr create(const SensorTime& st);

  /*! Same as create, only difference is return type. */
  EditDataPtr createE(const SensorTime& st)
    { return std::static_pointer_cast<EditData>(create(st)); }

  /*! Calls find, and if 0-pointer is returned, calls create. */
  EditDataPtr findOrCreateE(const SensorTime& st)
    { ObsDataPtr obs = find(st); if (not obs) obs = create(st); return std::static_pointer_cast<EditData>(obs); }

  virtual bool update(const std::vector<ObsUpdate>& updates);

  /*! Obtain an editor for the given observation data.
  * \param obs observation data, must belong to this EditAccess object
  */
  EditDataEditorPtr editor(EditDataPtr obs);

  /*! Commit changes made in the editor. */
  bool commit(EditDataEditor* editor);

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
  int countT() const
    { return mTasks; }

  const timeutil::ptime& versionTimestamp(int version) const
    { return mVersionTimestamps[version]; }

  typedef std::vector<EditDataPtr> ChangedData_t;
  ChangedData_t versionChanges(int version) const;

  boost::signal2<void, int, int> currentVersionChanged;

  bool sendChangesToParent(bool alsoSendTasks=true);
  void reset();

  boost::signal2<void, ObsDataChange, EditDataPtr> backendDataChanged;

private:
  void sendObsDataChanged(ObsDataChange what, ObsDataPtr obs, int dUpdated, int dTasks);
  void onBackendDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);
  void updateToCurrentVersion(bool drop);
  void addEditTimes(TimeSet& times, const std::vector<Sensor>& sensors, const TimeRange& limits);

private:
  ObsAccessPtr mBackend;
  typedef std::map<SensorTime, EditDataPtr, lt_SensorTime> Data_t;
  Data_t mData;

  typedef std::vector<timeutil::ptime> VersionTimestamps_t;
  VersionTimestamps_t mVersionTimestamps;
  int mCurrentVersion, mUpdated, mTasks;
};
typedef std::shared_ptr<EditAccess> EditAccessPtr;

#endif // EditAccess_hh
