
#ifndef TimeRangeControl_hh
#define TimeRangeControl_hh 1

#include "TimeRange.hh"
#include <QtCore/QObject>

class MiDateTimeEdit;

class TimeRangeControl : public QObject
{ Q_OBJECT;
public:
  TimeRangeControl(QObject* parent=0);
  void setMinimumGap(int hours);
  void install(MiDateTimeEdit* t0, MiDateTimeEdit* t1);

  TimeRange timeRange() const;
                                      
private Q_SLOTS:
  void onT0TimeChanged(const QTime&);
  void onT1TimeChanged(const QTime&);
  void onT0DateChanged(const QDate&);
  void onT1DateChanged(const QDate&);

private:
  QDateTime addGap(const QDateTime& t, int sign);
  void changedT0();
  void changedT1();
  
private:
  MiDateTimeEdit* mT0;
  MiDateTimeEdit* mT1;
  int mMinimumGap;
};

#endif // TimeRangeControl_hh
