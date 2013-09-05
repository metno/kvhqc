
#ifndef DATAVXITEM_HH
#define DATAVXITEM_HH 1

#include "DataValueItem.hh"

class DataVxItem : public DataValueItem {
public:
  DataVxItem(ObsColumn::Type columnType, EditAccessPtr da);

  virtual QVariant data(EditDataPtr obs, const SensorTime& st, int role) const;
  virtual bool setData(EditDataPtr obs, EditAccessPtr da, const SensorTime& st, const QVariant& value, int role);
  virtual QString description(bool mini) const;
  virtual bool matchSensor(const Sensor& sensorColumn, const Sensor& sensorObs) const;

private:
  typedef std::pair<int,int> Codes_t;
  Codes_t getCodes(EditDataPtr obs1, EditDataPtr obs2) const;
  EditDataPtr getObs2(EditDataPtr obs1) const;
  Sensor getSensor2(const Sensor& sensor1) const;
  
private:
  EditAccessPtr mDA;
};

#endif // DATAVXITEM_HH
