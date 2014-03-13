
#ifndef COMMON_NAVIGATEHELPER_HH
#define COMMON_NAVIGATEHELPER_HH 1

#include "Sensor.hh"
#include <boost/noncopyable.hpp>

class NavigateHelper
{
public:
  NavigateHelper();
  ~NavigateHelper();

  class Locker : boost::noncopyable {
  public:
    Locker(NavigateHelper& nh)
      : mNavigateHelper(nh) { nh.lock(); }
    ~Locker()
      { mNavigateHelper.unlock(); }
  private:
    NavigateHelper& mNavigateHelper;
  };

  bool go(const SensorTime& st);

  void lock()
    { mLocked += 1; }

  void unlock()
    { mLocked -= 1; }

  bool locked() const
    { return mLocked > 1; }

  bool changed() const
    { return mChanged; }

  bool invalidate();

  const SensorTime& current() const
    { return mLastNavigated; }

private:
  SensorTime mLastNavigated;
  size_t mLocked;
  bool mChanged;
};

#endif // COMMON_NAVIGATEHELPER_HH
