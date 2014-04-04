
#ifndef DATAVXITEM_HH
#define DATAVXITEM_HH 1

#include "DataValueItem.hh"

class DataVxItem : public DataValueItem {
public:
  DataVxItem(ObsColumn::Type columnType, EditAccess_p da);

  virtual QVariant data(ObsData_p obs, const SensorTime& st, int role) const;
  virtual bool setData(ObsData_p obs, EditAccess_p da, const SensorTime& st, const QVariant& value, int role);
  virtual QString description(bool mini) const;
  virtual bool matchSensor(const Sensor& sensorColumn, const Sensor& sensorObs) const;

private:
  typedef std::pair<int,int> Codes_t;
  Codes_t getCodes(ObsData_p obs1, ObsData_p obs2) const;
  ObsData_p getObs2(ObsData_p obs1) const;
  Sensor getSensor2(const Sensor& sensor1) const;
  
private:
  EditAccess_p mDA;
};

#endif // DATAVXITEM_HH
