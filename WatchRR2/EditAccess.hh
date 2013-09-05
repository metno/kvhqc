
#ifndef EditAccess_hh
#define EditAccess_hh 1

#include "EditDataEditor.hh"
#include "ObsAccess.hh"

class EditAccess : public ObsAccess {
public:
    EditAccess(ObsAccessPtr backend);
    virtual ~EditAccess();

    virtual TimeSet allTimes(const Sensor& sensor, const TimeRange& limits);

    virtual ObsDataPtr find(const SensorTime& st);
    EditDataPtr findE(const SensorTime& st)
        { return boost::static_pointer_cast<EditData>(find(st)); }

    virtual ObsDataPtr create(const SensorTime& st);
    EditDataPtr createE(const SensorTime& st)
        { return boost::static_pointer_cast<EditData>(create(st)); }

    EditDataPtr findOrCreateE(const SensorTime& st)
        { ObsDataPtr obs = find(st); if (not obs) obs = create(st); return boost::static_pointer_cast<EditData>(obs); }

    virtual bool update(const std::vector<ObsUpdate>& updates);

    virtual void addSubscription(const ObsSubscription& s)
        { mBackend->addSubscription(s); }

    virtual void removeSubscription(const ObsSubscription& s)
        { mBackend->removeSubscription(s); }

    EditDataEditorPtr editor(EditDataPtr obs);
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

    bool sendChangesToParent();
    void reset();

    boost::signal2<void, ObsDataChange, EditDataPtr> backendDataChanged;

private:
    void sendObsDataChanged(ObsDataChange what, ObsDataPtr obs, int dUpdated, int dTasks);
    void onBackendDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);
    void updateToCurrentVersion(bool drop);

private:
    ObsAccessPtr mBackend;
    typedef std::map<SensorTime, EditDataPtr, lt_SensorTime> Data_t;
    Data_t mData;

    typedef std::vector<timeutil::ptime> VersionTimestamps_t;
    VersionTimestamps_t mVersionTimestamps;
    int mCurrentVersion, mUpdated, mTasks;
};
typedef boost::shared_ptr<EditAccess> EditAccessPtr;

#endif // EditAccess_hh
