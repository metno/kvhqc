
#include "NavigateHelper.hh"

NavigateHelper::NavigateHelper()
  : mLocked(0)
  , mChanged(false)
{
}

NavigateHelper::~NavigateHelper()
{
}

bool NavigateHelper::go(const SensorTime& st)
{
  if (not st.valid() or eq_SensorTime()(mLastNavigated, st))
    return false;

  mLastNavigated = st;
  mChanged = true;
  
  return not locked();
}

bool NavigateHelper::invalidate()
{
  if (not mLastNavigated.valid())
    return false;

  mLastNavigated = SensorTime();
  mChanged = true;
  
  return not locked();
}
