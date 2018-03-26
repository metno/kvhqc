
#include "StationCardModel.hh"

#include "TasksColumn.hh"
#include "common/ColumnFactory.hh"
#include "common/ModelColumn.hh"
#include "common/KvHelpers.hh"
#include "util/Helpers.hh"

#define MILOGGER_CATEGORY "kvhqc.StationCardModel"
#include "common/ObsLogging.hh"

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

StationCardModel::StationCardModel(TaskAccess_p da, ModelAccess_p ma, const Sensor& sensor, const TimeSpan& time)
  : WatchRRTableModel(da)
{
  setTimeSpan(time);

  for(int i=0; i<N_COLUMNS; ++i) {
    const Sensor s(sensor.stationId, columnPars[i], sensor.level, sensor.sensor, sensor.typeId);

    DataColumn_p dc = ColumnFactory::columnForSensor(da, s, time, columnTypes[i]);
    if (not dc) {
      HQC_LOG_WARN("no column for sensor " << s);
      continue;
    }

    dc->setHeaderShowStation(false);
    if (columnTimeOffsets[i] != 0)
      dc->setTimeOffset(boost::posix_time::hours(columnTimeOffsets[i]));

    if (i == getRR24CorrectedColumn()) {
      mRR24EditTime = std::make_shared<EditTimeColumn>(dc);
      addColumn(mRR24EditTime);
    } else {
      addColumn(std::make_shared<TasksColumn>(dc));
    }
  }

  if (ma) {
    ModelColumn_p mc = ColumnFactory::columnForSensor(ma, sensor, time);
    mc->setHeaderShowStation(false);
    addColumn(mc);
  }
}

int StationCardModel::getRR24CorrectedColumn() const
{
  // must match columns as listed above
  return 9;
}

int StationCardModel::getRR24OriginalColumn() const
{
  // must match columns as listed above
  return getRR24CorrectedColumn() + 3;
}

void StationCardModel::setRR24TimeSpan(const TimeSpan& tr)
{
  mRR24EditTime->setEditableTime(tr);
}
