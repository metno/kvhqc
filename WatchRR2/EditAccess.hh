
#ifndef EditAccess_hh
#define EditAccess_hh 1

#include "EditDataEditor.hh"
#include "ObsAccess.hh"

class EditAccess : public ObsAccess {
public:
    EditAccess(ObsAccessPtr backend);
    virtual ~EditAccess();

    virtual ObsDataPtr find(const SensorTime& st);
    EditDataPtr findE(const SensorTime& st)
        { return boost::static_pointer_cast<EditData>(find(st)); }

    virtual ObsDataPtr create(const SensorTime& st);
    EditDataPtr createE(const SensorTime& st)
        { return boost::static_pointer_cast<EditData>(create(st)); }

    EditDataPtr findOrCreateE(const SensorTime& st)
        { ObsDataPtr obs = find(st); if (not obs) obs = create(st); return boost::static_pointer_cast<EditData>(obs); }

    virtual bool update(const std::vector<ObsUpdate>& updates);

    EditDataEditorPtr editor(EditDataPtr obs);
    int currentUpdate() const
        { return mUpdateCount; }
    void pushUpdate();
    bool popUpdate();

    bool sendChangesToParent();

    int countUpdates() const { return mUpdateCount; }

    int countU() const { return mUpdated; }
    int countT() const { return mTasks; }

    void sendObsDataChanged(ObsDataChange what, ObsDataPtr obs, int dUpdated, int dTasks);

private:
    void onBackendDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);

private:
    ObsAccessPtr mBackend;
    typedef std::map<SensorTime, EditDataPtr, lt_SensorTime> Data_t;
    Data_t mData;
    int mUpdateCount, mUpdated, mTasks;
};
typedef boost::shared_ptr<EditAccess> EditAccessPtr;

#endif // EditAccess_hh
