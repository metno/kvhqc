
#ifndef StaticDataList_hh
#define StaticDataList_hh 1

#include "DataList.hh"
#include "ObsColumn.hh"

class StaticDataList : public DataList
{ Q_OBJECT
public:
  StaticDataList(QWidget* parent=0);
  ~StaticDataList();
  
  virtual void setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits);
};

#endif // DataList_hh
