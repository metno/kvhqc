
#ifndef StationDataList_hh
#define StationDataList_hh 1

#include "TimespanDataList.hh"

class ObsPgmRequest;

class StationDataList : public TimespanDataList
{ Q_OBJECT
public:
  StationDataList(QWidget* parent=0);
  ~StationDataList();

protected:
  std::string viewType() const;
  void doSensorSwitch();
  SensorTime sensorSwitch() const;
  void updateModel();

private Q_SLOTS:
  void onObsPgmsComplete();

private:
  void addSensorColumns(const Sensor& s);

private:
  ObsPgmRequest* mObsPgmRequest;
};

#endif // StationDataList_hh
