
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
  const bool changed = mChanged
      || (st.valid() != mLastNavigated.valid())
      || (not eq_SensorTime()(st, mLastNavigated));
  mLastNavigated = st;
  
  if (not blocked()) {
    mChanged = false;
    return changed;
  } else {
    mChanged = changed;
    return false;
  }
}

bool NavigateHelper::unblock()
{
  if (mBlocked>0)
    mBlocked -= 1;

  if (not blocked())
    return mChanged;
  else
    return 0;
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
