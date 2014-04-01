
#include "EditData.hh"

#include "Tasks.hh"
#include "util/Helpers.hh"

#define MILOGGER_CATEGORY "kvhqc.EditData"
#include "util/HqcLogging.hh"

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
  METLIBS_LOG_SCOPE();
  
  const float bOriginal = mData->original();
  const bool changedO = (not Helpers::float_eq()(mOriginal, bOriginal));
  mOriginal = bOriginal;

  const bool changedC = mCorrected.changeOriginal(mData->corrected());
  const bool changedI = mControlinfo.changeOriginal(mData->controlinfo());
  const bool changedT = mTasks.changeOriginal(tasksFrom(mData));

  METLIBS_LOG_DEBUG("changed o=" << changedO << " c=" << changedC << " i=" << changedI << " t=" << changedT);

  return (changedO or changedC or changedI or changedT);
}
