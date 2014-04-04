
#ifndef HQC_STATICDATALIST_HH
#define HQC_STATICDATALIST_HH 1

#include "DataList.hh"
#include "common/ObsColumn.hh"

class StaticDataList : public DataList
{ Q_OBJECT
public:
  StaticDataList(QWidget* parent=0);
  ~StaticDataList();
  
  virtual void setSensorsAndTimes(const Sensor_v& sensors, const TimeSpan& limits);
};

#endif // HQC_STATICDATALIST_HH
