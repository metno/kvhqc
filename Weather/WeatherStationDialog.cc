
#include "WeatherStationDialog.hh"

#include "WeatherTableModel.hh"
#include <kvalobs/kvObsPgm.h>
#include <algorithm>

#define MILOGGER_CATEGORY "kvhqc.WeatherStationDialog"
#include "HqcLogging.hh"

WeatherStationDialog::WeatherStationDialog(const Sensor& sensor, const TimeRange& time, QDialog* parent)
  : StationDialog(sensor, time, parent)
{
  setWindowTitle(tr("WatchWeather -- Station"));
  onUpdateType();
}

WeatherStationDialog::WeatherStationDialog(QDialog* parent)
  : StationDialog(parent)
{
  setWindowTitle(tr("WatchWeather -- Station"));
  onUpdateType();
}

bool WeatherStationDialog::acceptThisObsPgm(const kvalobs::kvObsPgm& op) const
{
  METLIBS_LOG_SCOPE();
  if (not (op.kl06() or op.kl07() or op.collector()))
    return false;

  const int *pb = WeatherTableModel::parameters, *pe = pb + WeatherTableModel::NPARAMETERS;
  if (std::find(pb, pe, op.paramID()) == pe) {
    METLIBS_LOG_DEBUG("parameter " << op.paramID() << " not in weather table");
    return false;
  }

  return true;
}
