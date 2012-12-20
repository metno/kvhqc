
#ifndef EditData_hh
#define EditData_hh 1

#include "ObsData.hh"
#include <vector>

class EditData;
typedef boost::shared_ptr<EditData> EditDataPtr;

class EditData : public ObsData {
public:
    EditData(ObsDataPtr data)
        : mData(data), mCreated(false), mOldTasks(0) { }

    virtual SensorTime sensorTime() const
        { return mData->sensorTime(); }

    virtual float original() const { return mData->original(); }

    virtual float corrected() const { return mNewCorrected.empty() ? oldCorrected() : mNewCorrected.back().second; }
    virtual kvalobs::kvControlInfo controlinfo() const { return mNewControlinfo.empty() ? oldControlinfo() : mNewControlinfo.back().second; }

    float oldCorrected() const { return mData->corrected(); }
    kvalobs::kvControlInfo oldControlinfo() const { return mData->controlinfo(); }

    bool hasTasks() const { return allTasks() != 0; }
    bool hasTask(int id) const { return (allTasks() & (1<<id)) != 0; }
    int allTasks() const { return mTasks.empty() ? mOldTasks : mTasks.back().second; }
    bool modified() const;
    bool modifiedTasks() const
        { return mOldTasks != allTasks(); }
    bool created() const
        { return mCreated; }

private:
    ObsDataPtr mData;
    bool mCreated;
    int mOldTasks;
    std::vector< std::pair<int,float> > mNewCorrected;
    std::vector< std::pair<int,kvalobs::kvControlInfo> > mNewControlinfo;
    std::vector< std::pair<int,int> > mTasks;

    friend class EditAccess;
    friend class EditDataEditor;
};

#endif // EditData_hh
