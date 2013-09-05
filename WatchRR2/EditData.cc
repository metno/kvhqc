
#include "EditData.hh"

#include "Helpers.hh"
#include "Tasks.hh"

namespace /* anonymous */ {
int tasksFrom(ObsDataPtr obs)
{
    if (EditDataPtr bebs = boost::dynamic_pointer_cast<EditData>(obs))
        return bebs->allTasks();
    else
        return 0;
}
} // anonymous namespace

EditData::EditData(ObsDataPtr data)
    : mData(data)
    , mCreated(false)
    , mOriginal(mData->original())
    , mCorrected(mData->corrected())
    , mControlinfo(mData->controlinfo())
    , mTasks(tasksFrom(mData))
{
}

bool EditData::hasRequiredTasks() const
{
  return (allTasks() & tasks::REQUIRED_TASK_MASK) != 0;
}

void EditData::reset()
{
    mCorrected.reset(mData->corrected());
    mControlinfo.reset(mData->controlinfo());
    mTasks.reset(tasksFrom(mData));
}

bool EditData::updateFromBackend()
{
    const float bOriginal = mData->original();
    bool changed = (not Helpers::float_eq()(mOriginal, bOriginal));
    mOriginal = bOriginal;

    changed |= mCorrected.changeOriginal(mData->corrected());
    changed |= mControlinfo.changeOriginal(mData->controlinfo());
    changed |= mTasks.changeOriginal(tasksFrom(mData));

    return changed;
}
