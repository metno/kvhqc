
#include "TimeRangeControl.hh"

#include "MiDateTimeEdit.hh"

TimeRangeControl::TimeRangeControl(QObject* parent)
  : QObject(parent)
  , mT0(0)
  , mT1(0)
  , mMinimumGap(24)
{
}

void TimeRangeControl::setMinimumGap(int hours)
{
  mMinimumGap = hours;
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

QDateTime TimeRangeControl::addGap(const QDateTime& t, int sign)
{
  return t.addSecs(((sign>=0) ? 1 : -1) * 3600 * mMinimumGap);
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
  const QDateTime t1min = addGap(mT0->dateTime(), 1);
  if (t1min > mT1->dateTime())
    mT1->setDateTime(t1min);
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
  const QDateTime t0max = addGap(mT1->dateTime(), -1);
  if (mT0->dateTime() > t0max)
    mT0->setDateTime(t0max);
}
