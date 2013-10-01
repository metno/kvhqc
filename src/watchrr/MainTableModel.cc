
#include "MainTableModel.hh"

#include "common/ColumnFactory.hh"
#include "common/ModelColumn.hh"
#include "util/Helpers.hh"

#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.MainTableModel"
#include "util/HqcLogging.hh"

namespace /* anonymous */ {
const int N_COLUMNS = 18;
const int columnPars[N_COLUMNS] = {
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD
};
const ObsColumn::Type columnTypes[N_COLUMNS] = {
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,   ObsColumn::NEW_CORRECTED,
  ObsColumn::ORIGINAL,        ObsColumn::ORIGINAL,        ObsColumn::ORIGINAL,
  ObsColumn::NEW_CONTROLINFO, ObsColumn::NEW_CONTROLINFO, ObsColumn::NEW_CONTROLINFO
};
const int columnTimeOffsets[N_COLUMNS] = {
  -18, -18, -18,
  -12, -12, -12,
  0, 0, 0,
  0, 0, 0,
  0, 0, 0,
  0, 0, 0
};
} // namespace anonymous

MainTableModel::MainTableModel(EditAccessPtr da, ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time)
  : ObsTableModel(da, time)
{
  for(int i=0; i<N_COLUMNS; ++i) {
    const Sensor s(sensor.stationId, columnPars[i], sensor.level, sensor.sensor, sensor.typeId);
    DataColumnPtr oc = ColumnFactory::columnForSensor(da, s, time, columnTypes[i]);
    oc->setHeaderShowStation(false);
    if (columnTimeOffsets[i] != 0)
      oc->setTimeOffset(boost::posix_time::hours(columnTimeOffsets[i]));
    if (i == getRR24Column()) {
      mRR24EditTime = boost::make_shared<EditTimeColumn>(oc);
      addColumn(mRR24EditTime);
    } else {
      addColumn(oc);
    }
  }

  if (ma) {
    ModelColumnPtr mc = ColumnFactory::columnForSensor(ma, sensor, time);
    mc->setHeaderShowStation(false);
    addColumn(mc);
  }
}

int MainTableModel::getRR24Column() const
{
  // must match columns as listed above
  return 9;
}

void MainTableModel::setRR24TimeRange(const TimeRange& tr)
{
  mRR24EditTime->setEditableTime(tr);
}
