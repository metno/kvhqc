
#include "StaticDataList.hh"

#include "BusyIndicator.h"
#include "ColumnFactory.hh"
#include "DataListModel.hh"

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.StaticDataList"
#include "HqcLogging.hh"

StaticDataList::StaticDataList(QWidget* parent)
  : DataList(parent)
{
}

StaticDataList::~StaticDataList()
{
}

void StaticDataList::setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits)
{
  DataView::setSensorsAndTimes(sensors, limits);

  BusyIndicator busy;
  std::auto_ptr<DataListModel> newModel(new DataListModel(mDA, limits));

  BOOST_FOREACH(const Sensor& s, sensors) {
    DataColumnPtr oc = ColumnFactory::columnForSensor(mDA, s, limits, ObsColumn::ORIGINAL);
    if (oc)
      newModel->addColumn(oc);

    DataColumnPtr cc = ColumnFactory::columnForSensor(mDA, s, limits, ObsColumn::NEW_CORRECTED);
    if (cc)
      newModel->addColumn(cc);
  }

  updateModel(newModel.release());
}
