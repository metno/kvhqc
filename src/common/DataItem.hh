
#ifndef DATAITEM_HH
#define DATAITEM_HH 1

#include "access/EditAccess.hh"
#include "ObsColumn.hh"

class DataItem {
public:
  virtual ~DataItem();

  virtual Qt::ItemFlags flags(ObsData_p obs) const;
  virtual QVariant data(ObsData_p obs, const SensorTime& st, int role) const;
  virtual bool setData(ObsData_p obs, EditAccess_p ea, const SensorTime& st, const QVariant& value, int role);
  virtual QString description(bool mini) const = 0;
  virtual bool matchSensor(const Sensor& sensorColumn, const Sensor& sensorObs) const;
  virtual int type() const = 0;
};

HQC_TYPEDEF_P(DataItem);

#endif // DATAITEM_HH
