
#include "EditDataEditor.hh"
#include "EditAccess.hh"
#include "FlagChange.hh"

#define NDEBUG
#include "w2debug.hh"

EditDataEditor::EditDataEditor(EditAccess* ea, EditDataPtr obs)
    : mEA(ea)
    , mObs(obs)
    , mCorrected(obs->corrected())
    , mControlinfo(obs->controlinfo())
    , mTasks(obs->allTasks())
{
}

EditDataEditor::~EditDataEditor()
{
    commit();
}

bool EditDataEditor::commit()
{
    const int wasModified = mObs->modified()?1:0, hadTasks = mObs->hasTasks()?1:0;
    const bool changed = applyChanges();
    if (changed) {
        const int isModified = mObs->modified()?1:0, hasTasks = mObs->hasTasks()?1:0;
        mEA->sendObsDataChanged(EditAccess::MODIFIED, mObs, isModified - wasModified, hasTasks - hadTasks);
    }
    return changed;
}

bool EditDataEditor::applyChanges()
{
    const int u = mEA->currentUpdate();
    bool changed = false;
    if (mCorrected.commit()) {
        mObs->mCorrected.setValue(u, mCorrected.get());
        changed = true;
    }
    if (mControlinfo.commit()) {
        mObs->mControlinfo.setValue(u, mControlinfo.get());
        changed = true;
    }
    if (mTasks.commit()) {
        mObs->mTasks.setValue(u, mTasks.get());
        changed = true;
    }
    return changed;
}

EditDataEditor& EditDataEditor::setCorrected(float nc)
{
    mCorrected.set(nc);
    return *this;
}

EditDataEditor& EditDataEditor::setControlinfo(const kvalobs::kvControlInfo& nci)
{
    mControlinfo.set(nci);
    return *this;
}

EditDataEditor& EditDataEditor::changeControlinfo(const FlagChange& fc)
{
    return setControlinfo(fc.apply(mControlinfo.get()));
}

EditDataEditor& EditDataEditor::addTask(int id)
{
    mTasks.set(mTasks.get() | (1<<id));
    return *this;
}

EditDataEditor& EditDataEditor::clearTask(int id)
{
    mTasks.set(mTasks.get() & ~(1<<id));
    return *this;
}

EditDataEditor& EditDataEditor::clearTasks(int tasks)
{
    mTasks.set(mTasks.get() & ~tasks);
    return *this;
}

EditDataEditor& EditDataEditor::setTasks(int tasks)
{
    mTasks.set(tasks);
    return *this;
}
