
#ifndef TimeSpanControl_hh
#define TimeSpanControl_hh 1

#include "common/TimeSpan.hh"
#include <QtCore/QObject>

class MiDateTimeEdit;

class TimeSpanControl : public QObject
{ Q_OBJECT;
public:
  TimeSpanControl(QObject* parent=0);
  void setMinimumGap(int hours);
  void setMaximumGap(int hours);
  void install(MiDateTimeEdit* t0, MiDateTimeEdit* t1);

  TimeSpan timeRange() const;
                                      
private Q_SLOTS:
  void onT0TimeChanged(const QTime&);
  void onT1TimeChanged(const QTime&);
  void onT0DateChanged(const QDate&);
  void onT1DateChanged(const QDate&);

private:
  void changedT0();
  void changedT1();
  
private:
  MiDateTimeEdit* mT0;
  MiDateTimeEdit* mT1;
  int mMinimumGap;
  int mMaximumGap;
};

#endif // TimeSpanControl_hh
