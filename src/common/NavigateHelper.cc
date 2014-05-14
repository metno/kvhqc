
#include "NavigateHelper.hh"

NavigateHelper::NavigateHelper()
  : mBlocked(0)
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
  
  return not blocked();
}

bool NavigateHelper::unblock()
{
  if (mBlocked>0)
    mBlocked -= 1;
  return mLastNavigated.valid() and not blocked();
}

bool NavigateHelper::invalidate()
{
  if (not mLastNavigated.valid())
    return false;

  mLastNavigated = SensorTime();
  mChanged = true;
  
  return not blocked();
}

// ########################################################################

bool NavigateVisible::updateVisible(bool visible)
{
  if (visible and not mVisible) { // show
    mVisible = true;
    return unblock();
  } else if (mVisible and not visible) { // hide
    mVisible = false;
    block();
  }
  return false;
}
