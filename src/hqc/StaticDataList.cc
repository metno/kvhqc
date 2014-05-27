
#include "StaticDataList.hh"

#include "common/ColumnFactory.hh"
#include "common/DataListModel.hh"
#include "util/BusyIndicator.hh"

#define MILOGGER_CATEGORY "kvhqc.StaticDataList"
#include "util/HqcLogging.hh"

StaticDataList::StaticDataList(QWidget* parent)
  : DataList(parent)
{
}

StaticDataList::~StaticDataList()
{
}

void StaticDataList::setSensorsAndTimes(const Sensor_v& sensors, const TimeSpan& limits)
{
  BusyIndicator busy;

  model()->removeAllColumns();

  for (Sensor_v::const_iterator it = sensors.begin(); it != sensors.end(); ++it) {
    DataColumn_p oc = ColumnFactory::columnForSensor(mDA, *it, limits, ObsColumn::ORIGINAL);
    if (oc)
      model()->addColumn(oc);

    DataColumn_p cc = ColumnFactory::columnForSensor(mDA, *it, limits, ObsColumn::NEW_CORRECTED);
    if (cc)
      model()->addColumn(cc);
  }
}
