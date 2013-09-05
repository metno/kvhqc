
#include "EditDataEditor.hh"
#include "EditAccess.hh"
#include "FlagChange.hh"

#define MILOGGER_CATEGORY "kvhqc.EditDataEditor"
#include "HqcLogging.hh"

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
    return mEA->commit(this);
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
