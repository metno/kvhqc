
#ifndef COMMON_NAVIGATEHELPER_HH
#define COMMON_NAVIGATEHELPER_HH 1

#include "Sensor.hh"

class NavigateHelper
{
public:
  NavigateHelper();
  ~NavigateHelper();

  class Blocker : boost::noncopyable {
  public:
    Blocker(NavigateHelper& nh)
      : mNavigateHelper(nh) { nh.block(); }
    ~Blocker()
      { mNavigateHelper.unblock(); }
  private:
    NavigateHelper& mNavigateHelper;
  };

  bool go(const SensorTime& st);

  void block()
    { mBlocked += 1; }

  bool unblock();

  bool blocked() const
    { return mBlocked > 1; }

  bool changed() const
    { return mChanged; }

  bool invalidate();

  const SensorTime& current() const
    { return mLastNavigated; }

private:
  SensorTime mLastNavigated;
  size_t mBlocked;
  bool mChanged;
};

// ########################################################################

class NavigateVisible : public NavigateHelper {
public:
  NavigateVisible(bool initiallyVisible=true) : mVisible(initiallyVisible) { }

  /** Update visibility, return true if navigtion should be performed.
   *  Never returns true if visible is false.
   *
   *  Needs to be called from showEvent with true, hideEvent with false, and resizeEvent with 
   *  <code> (not re->size().isEmpty()) </code>.
   */
  bool updateVisible(bool visible);
  
private:
  bool mVisible;
};

#endif // COMMON_NAVIGATEHELPER_HH
