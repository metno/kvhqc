
#include "NeighborRR24Model.hh"

#include "common/ColumnFactory.hh"
#include "common/KvHelpers.hh"
#include "common/NeighborHeader.hh"
#include "common/ObsPgmRequest.hh"

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.NeighborRR24Model"
#include "util/HqcLogging.hh"

NeighborRR24Model::NeighborRR24Model(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time)
    : WatchRRTableModel(da)
    , mNeighbors(1, sensor)
{
  setTimeSpan(time);

  hqc::int_s stationIds = KvMetaDataBuffer::instance()->findNeighborStationIds(sensor.stationId);
  stationIds.insert(sensor.stationId);

  std::auto_ptr<ObsPgmRequest> op(new ObsPgmRequest(stationIds));
  op->sync();
  
  KvMetaDataBuffer::instance()->addNeighbors(mNeighbors, sensor, time, op.get(), 20);
  BOOST_FOREACH(const Sensor& s, mNeighbors) {
    if (DataColumn_p oc = ColumnFactory::columnForSensor(da, s, time, ObsColumn::ORIGINAL))
      addColumn(oc);
  }
}

QVariant NeighborRR24Model::columnHeader(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::ToolTipRole)
    return ObsTableModel::columnHeader(section, orientation, role);
  
  return NeighborHeader::headerData(mNeighbors[0].stationId, mNeighbors[section].stationId, orientation, role);
}
