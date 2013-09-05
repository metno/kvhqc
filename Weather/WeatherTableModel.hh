
#ifndef WEATHERTABLEMODEL_HH
#define WEATHERTABLEMODEL_HH 1

#include "ColumnFactory.hh"
#include "ObsTableModel.hh"

class WeatherTableModel : public ObsTableModel
{
public:
  WeatherTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, ObsColumn::Type t);

  virtual timeutil::ptime timeAtRow(int row) const;
  
protected:
  virtual int rowAtTime(const timeutil::ptime& time) const;
  virtual int rowOrColumnCount(bool timeDirection) const;
};

#endif /* WEATHERTABLEMODEL_HH */
