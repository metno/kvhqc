
#include "StaticDataList.hh"

#include "common/ColumnFactory.hh"
#include "common/DataListModel.hh"
#include "util/BusyIndicator.hh"

#include <boost/foreach.hpp>

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
  std::auto_ptr<DataListModel> newModel(new DataListModel(mDA, limits));

  BOOST_FOREACH(const Sensor& s, sensors) {
    DataColumn_p oc = ColumnFactory::columnForSensor(mDA, s, limits, ObsColumn::ORIGINAL);
    if (oc)
      newModel->addColumn(oc);

    DataColumn_p cc = ColumnFactory::columnForSensor(mDA, s, limits, ObsColumn::NEW_CORRECTED);
    if (cc)
      newModel->addColumn(cc);
  }

  updateModel(newModel.release());
}
