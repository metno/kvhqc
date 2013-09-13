
#include "NeighborTableModel.hh"

#include "common/ColumnFactory.hh"
#include "common/gui/NeighborHeader.hh"

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.NeighborTableModel"
#include "util/HqcLogging.hh"

NeighborTableModel::NeighborTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
    : ObsTableModel(da, time)
    , mNeighbors(1, sensor)
{
  Helpers::addNeighbors(mNeighbors, sensor, time, 20);
  BOOST_FOREACH(const Sensor& s, mNeighbors) {
    DataColumnPtr oc = ColumnFactory::columnForSensor(da, s, time, ObsColumn::ORIGINAL);
    if (oc)
      addColumn(oc);
  }
}

QVariant NeighborTableModel::columnHeader(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::ToolTipRole)
    return ObsTableModel::columnHeader(section, orientation, role);
  
  return NeighborHeader::headerData(mNeighbors[0].stationId, mNeighbors[section].stationId, orientation, role);
}
