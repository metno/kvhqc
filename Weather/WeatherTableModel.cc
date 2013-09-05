
#include "WeatherTableModel.hh"

#define MILOGGER_CATEGORY "kvhqc.WeatherTableModel"
#include "HqcLogging.hh"

namespace /* anonymous */ {
const int columnPars[] = {
  211,214,216,213,215,262,178,173,177,1,61,81,86,87,83,90,15,14,55,108,
  109,110,112,18,7,273,41,31,32,33,42,43,34,36,38,40,
  23,24,22,403,404,131,134,151,154,250,221,9,12
};
const int N_COLUMNS = sizeof(columnPars)/sizeof(columnPars[0]);
} // namespace anonymous

WeatherTableModel::WeatherTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, ObsColumn::Type t)
  : ObsTableModel(da, time)
{
  for(int i=0; i<N_COLUMNS; ++i) {
    const Sensor s(sensor.stationId, columnPars[i], sensor.level, sensor.sensor, sensor.typeId);
    DataColumnPtr oc = ColumnFactory::columnForSensor(da, s, time, t);
    if (oc) {
      oc->setHeaderShowStation(false);
      addColumn(oc);
    }
  }
}

int WeatherTableModel::rowOrColumnCount(bool timeDirection) const
{
  if (timeDirection == mTimeInRows)
    return mTime.hours() + 1;
  return ObsTableModel::rowOrColumnCount(timeDirection);
}

timeutil::ptime WeatherTableModel::timeAtRow(int row) const
{
  return mTime.t0() + boost::posix_time::hours(row);
}

int WeatherTableModel::rowAtTime(const timeutil::ptime& time) const
{
  const int r = (time - mTime.t0()).hours();
  if (timeAtRow(r) != time)
    return -1;
  else
    return r;
}
