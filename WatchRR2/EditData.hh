
#ifndef EditData_hh
#define EditData_hh 1

#include "ObsData.hh"
#include <vector>

class EditData;
typedef boost::shared_ptr<EditData> EditDataPtr;

class EditData : public ObsData {
public:
    EditData(ObsDataPtr data);

    virtual SensorTime sensorTime() const
        { return mData->sensorTime(); }

    virtual float original() const
        { return mOriginal; }

    virtual float corrected() const
        { return mCorrected.back().second; }

    virtual kvalobs::kvControlInfo controlinfo() const
        { return mControlinfo.back().second; }

    float oldCorrected() const
        { return mCorrected.front().second; }

    const kvalobs::kvControlInfo& oldControlinfo() const
        { return mControlinfo.front().second; }

    bool hasTasks() const
        { return allTasks() != 0; }

    bool hasTask(int id) const
        { return (allTasks() & (1<<id)) != 0; }

    int allTasks() const
        { return mTasks.back().second; }

    bool modified() const;

    bool modifiedTasks() const
        { return mTasks.front().second != allTasks(); }

    bool created() const
        { return mCreated; }

    bool updateFromBackend();

private:
    ObsDataPtr mData;
    bool mCreated;
    float mOriginal;
    std::vector< std::pair<int,float> > mCorrected;
    std::vector< std::pair<int,kvalobs::kvControlInfo> > mControlinfo;
    std::vector< std::pair<int,int> > mTasks;

    friend class EditAccess;
    friend class EditDataEditor;
};

#endif // EditData_hh
