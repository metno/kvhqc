
#ifndef StationDataList_hh
#define StationDataList_hh 1

#include "TimespanDataList.hh"

class StationDataList : public TimespanDataList
{
public:
  StationDataList(QWidget* parent=0);
  ~StationDataList();

protected:
  std::string viewType() const;
  SensorTime sensorSwitch() const;
  void updateModel();

private:
  void addSensorColumns(const Sensor& s);
};

#endif // StationDataList_hh
