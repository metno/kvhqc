
#ifndef EditDataEditor_hh
#define EditDataEditor_hh 1

#include "EditData.hh"
#include "Helpers.hh"
#include "ValueChange.hh"

class EditAccess;
class FlagChange;

class EditDataEditor {
private:
    EditDataEditor(EditAccess* ea, EditDataPtr obs);
public:
    ~EditDataEditor();
    EditDataEditor& setCorrected(float nc);
    EditDataEditor& setControlinfo(const kvalobs::kvControlInfo& nci);
    EditDataEditor& changeControlinfo(const FlagChange& fc);
    EditDataEditor& addTask(int id);
    EditDataEditor& clearTask(int id);
    EditDataEditor& clearTasks(int tasks);
    EditDataEditor& setTasks(int tasks);
    bool commit();

    EditDataPtr obs() const
        { return mObs; }

protected:
    virtual bool applyChanges();

private:
    EditAccess* mEA;
    EditDataPtr mObs;
    ValueChange<float, Helpers::float_eq> mCorrected;
    ValueChange<kvalobs::kvControlInfo> mControlinfo;
    ValueChange<int> mTasks;

    friend class EditAccess;
};
typedef boost::shared_ptr<EditDataEditor> EditDataEditorPtr;

#endif // EditDataEditor_hh