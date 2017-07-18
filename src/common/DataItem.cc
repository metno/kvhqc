
#include "DataItem.hh"
#include "ObsColumn.hh"

#include <QStringList>
#include <QVariant>

DataItem::~DataItem()
{
}

Qt::ItemFlags DataItem::flags(ObsData_p) const
{
  return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

QVariant DataItem::data(ObsData_p, const SensorTime&, int role) const
{
  if (role == ObsColumn::ValueTypeRole)
    return ObsColumn::Numerical;
  else
    return QVariant();
}


bool DataItem::setData(ObsData_p, EditAccess_p, const SensorTime&, const QVariant&, int)
{
  throw std::runtime_error("cannot set original value");
  return false;
}

bool DataItem::matchSensor(const Sensor& sensorColumn, const Sensor& sensorObs) const
{
  return eq_Sensor()(sensorColumn, sensorObs);
}
