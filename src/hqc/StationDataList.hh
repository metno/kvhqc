
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
  DataListModel* makeModel();

private:
  void addSensorColumns(DataListModel* model, const Sensor& s);
};

#endif // StationDataList_hh
