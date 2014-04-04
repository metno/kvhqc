
#ifndef WEATHERSTATIONDIALOG_HH
#define WEATHERSTATIONDIALOG_HH

#include "watchrr/StationDialog.hh"

class WeatherStationDialog : public StationDialog
{ Q_OBJECT;
public:
  WeatherStationDialog(const Sensor& sensor, const TimeSpan& time, QDialog* parent=0);
  WeatherStationDialog(QDialog* parent=0);

protected:
  virtual int acceptThisObsPgm(const kvalobs::kvObsPgm& op) const;
};

#endif // WEATHERSTATIONDIALOG_HH
