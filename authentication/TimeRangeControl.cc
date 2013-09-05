
#include "TimeRangeControl.hh"

#include "MiDateTimeEdit.hh"

#define MILOGGER_CATEGORY "kvhqc.TimeRangeControl"
#include "HqcLogging.hh"

TimeRangeControl::TimeRangeControl(QObject* parent)
  : QObject(parent)
  , mT0(0)
  , mT1(0)
  , mMinimumGap(24)
  , mMaximumGap(0)
{
}

void TimeRangeControl::setMinimumGap(int hours)
{
  mMinimumGap = hours;
  if (mMinimumGap > 0 and mMaximumGap > 0 and mMinimumGap > mMaximumGap) {
    HQC_LOG_WARN("minimum gap (" << mMinimumGap << ") > maximum gap (" << mMaximumGap << ")");
    mMaximumGap = mMinimumGap;
  }
  if (mT0 and mT1)
    changedT1();
}

void TimeRangeControl::setMaximumGap(int hours)
{
  mMaximumGap = hours;
  if (mMinimumGap > 0 and mMaximumGap > 0 and mMinimumGap > mMaximumGap) {
    HQC_LOG_WARN("minimum gap (" << mMinimumGap << ") > maximum gap (" << mMaximumGap << ")");
    mMinimumGap = mMaximumGap;
  }
  if (mT0 and mT1)
    changedT1();
}

void TimeRangeControl::install(MiDateTimeEdit* t0, MiDateTimeEdit* t1)
{
  if (mT0 and mT1) {
    disconnect(mT0, SIGNAL(dateChanged(const QDate&)),
        this, SLOT(onT0DateChanged(const QDate&)));
    disconnect(mT0, SIGNAL(timeChanged(const QTime&)),
        this, SLOT(setMinTime(const QTime&)));
    
    disconnect(mT1, SIGNAL(dateChanged(const QDate&)),
        this,SLOT(onT1DateChanged(const QDate&) ));
    disconnect(mT1, SIGNAL(timeChanged(const QTime&)),
        this, SLOT(setMaxTime(const QTime&)));
  }

  mT0 = t0;
  mT1 = t1;

  if (mT0 and mT1) {
    connect(mT0, SIGNAL(dateChanged(const QDate&)),
        this, SLOT(onT0DateChanged(const QDate&)));
    connect(mT0, SIGNAL(timeChanged(const QTime&)),
        this, SLOT(onT0TimeChanged(const QTime&)));
    
    connect(mT1, SIGNAL(dateChanged(const QDate&)),
        this,SLOT(onT1DateChanged(const QDate&) ));
    connect(mT1, SIGNAL(timeChanged(const QTime&)),
        this, SLOT(onT1TimeChanged(const QTime&)));

    changedT1();
  }
}

TimeRange TimeRangeControl::timeRange() const
{
  const timeutil::ptime t0 = timeutil::from_QDateTime(mT0->dateTime());
  const timeutil::ptime t1 = timeutil::from_QDateTime(mT1->dateTime());
  return TimeRange(t0, t1);
}

void TimeRangeControl::onT0DateChanged(const QDate&)
{
  changedT0();
}

void TimeRangeControl::onT0TimeChanged(const QTime&)
{
  changedT0();
}

void TimeRangeControl::changedT0()
{
  if (mMinimumGap > 0) {
    const QDateTime t1min = mT0->dateTime().addSecs(3600*mMinimumGap);
    if (t1min > mT1->dateTime() and t1min <= mT1->maximumDateTime())
      mT1->setDateTime(t1min);
  }
  if (mMaximumGap > 0) {
    const QDateTime t1max = mT0->dateTime().addSecs(3600*mMaximumGap);
    if (t1max < mT1->dateTime() and t1max >= mT1->minimumDateTime())
      mT1->setDateTime(t1max);
  }
}

void TimeRangeControl::onT1DateChanged(const QDate&)
{
  changedT1();
}

void TimeRangeControl::onT1TimeChanged(const QTime&)
{
  changedT1();
}

void TimeRangeControl::changedT1()
{
  if (mMinimumGap > 0) {
    const QDateTime t0max = mT1->dateTime().addSecs(-3600*mMinimumGap);
    if (t0max < mT0->dateTime() and t0max >= mT0->minimumDateTime())
      mT0->setDateTime(t0max);
  }
  if (mMaximumGap > 0) {
    const QDateTime t0min = mT1->dateTime().addSecs(-3600*mMaximumGap);
    if (t0min > mT0->dateTime() and t0min <= mT0->maximumDateTime())
      mT0->setDateTime(t0min);
  }
}
