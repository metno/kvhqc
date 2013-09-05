
#ifndef WEATHERTABLEMODEL_HH
#define WEATHERTABLEMODEL_HH 1

#include "ColumnFactory.hh"
#include "DataListModel.hh"

class WeatherTableModel : public DataListModel
{
public:
  WeatherTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, ObsColumn::Type t);

  static const int NPARAMETERS;
  static const int parameters[];
};

#endif /* WEATHERTABLEMODEL_HH */
