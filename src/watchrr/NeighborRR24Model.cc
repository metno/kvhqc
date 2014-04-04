
#include "NeighborRR24Model.hh"

#include "common/ColumnFactory.hh"
#include "common/NeighborHeader.hh"

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.NeighborRR24Model"
#include "util/HqcLogging.hh"

NeighborRR24Model::NeighborRR24Model(EditAccessPtr da, const Sensor& sensor, const TimeSpan& time)
    : ObsTableModel(da, time)
    , mNeighbors(1, sensor)
{
  Helpers::addNeighbors(mNeighbors, sensor, time, 20);
  da->allData(mNeighbors, time);
  BOOST_FOREACH(const Sensor& s, mNeighbors) {
    DataColumnPtr oc = ColumnFactory::columnForSensor(da, s, time, ObsColumn::ORIGINAL);
    if (oc)
      addColumn(oc);
  }
}

QVariant NeighborRR24Model::columnHeader(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::ToolTipRole)
    return ObsTableModel::columnHeader(section, orientation, role);
  
  return NeighborHeader::headerData(mNeighbors[0].stationId, mNeighbors[section].stationId, orientation, role);
}
