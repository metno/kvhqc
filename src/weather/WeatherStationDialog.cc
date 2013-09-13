
#include "WeatherStationDialog.hh"

#include "WeatherTableModel.hh"
#include <kvalobs/kvObsPgm.h>
#include <algorithm>

#define MILOGGER_CATEGORY "kvhqc.WeatherStationDialog"
#include "util/HqcLogging.hh"

WeatherStationDialog::WeatherStationDialog(const Sensor& sensor, const TimeRange& time, QDialog* parent)
  : StationDialog(sensor, time, parent)
{
  setWindowTitle(tr("WatchWeather -- Station"));
  onEditStation();
}

WeatherStationDialog::WeatherStationDialog(QDialog* parent)
  : StationDialog(parent)
{
  setWindowTitle(tr("WatchWeather -- Station"));
  onEditStation();
}

int WeatherStationDialog::acceptThisObsPgm(const kvalobs::kvObsPgm& op) const
{
  METLIBS_LOG_SCOPE();

  const int pid = op.paramID(), tid = op.typeID();
  const int *pb = WeatherTableModel::parameters, *pe = pb + WeatherTableModel::NPARAMETERS;
  for (const int* p=pb; p!=pe; ++p) {
    if (*p == pid)
      return tid;
  }
  for (const int* p=pb; p!=pe; ++p) {
    if (Helpers::aggregatedParameter(pid, *p))
      return -tid;
  }
  return 0;
}
