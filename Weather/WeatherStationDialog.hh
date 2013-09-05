
#ifndef WEATHERSTATIONDIALOG_HH
#define WEATHERSTATIONDIALOG_HH

#include "StationDialog.hh"

class WeatherStationDialog : public StationDialog
{
public:
  WeatherStationDialog(const Sensor& sensor, const TimeRange& time, QDialog* parent=0);
  WeatherStationDialog(QDialog* parent=0);

protected:
  virtual bool acceptThisObsPgm(const kvalobs::kvObsPgm& op) const;
};

#endif // WEATHERSTATIONDIALOG_HH
