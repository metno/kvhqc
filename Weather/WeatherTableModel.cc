
#include "WeatherTableModel.hh"

#include "KvMetaDataBuffer.hh"

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.WeatherTableModel"
#include "HqcLogging.hh"

const int WeatherTableModel::parameters[] = {
  211,214,216,213,215,262,178,173,177,1,61,81,86,87,83,90,15,14,55,108,
  109,110,112,18,7,273,41,31,32,33,42,43,34,36,38,40,
  23,24,22,403,404,131,134,151,154,250,221,9,12
};
const int WeatherTableModel::NPARAMETERS = sizeof(parameters)/sizeof(parameters[0]);

WeatherTableModel::WeatherTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, ObsColumn::Type t)
  : DataListModel(da, time)
{
  const KvMetaDataBuffer::ObsPgmList& opl = KvMetaDataBuffer::instance()->findObsPgm(sensor.stationId);
  for(int i=0; i<NPARAMETERS; ++i) {
    BOOST_FOREACH(const kvalobs::kvObsPgm& op, opl) {
      if (time.intersection(TimeRange(op.fromtime(), op.totime())).undef())
        continue;
      const int paramId = parameters[i];
      if (paramId == op.paramID() and sensor.typeId == op.typeID()) {
        const Sensor s(sensor.stationId, paramId, sensor.level, sensor.sensor, sensor.typeId);
        DataColumnPtr oc = ColumnFactory::columnForSensor(da, s, time, t);
        if (oc) {
          oc->setHeaderShowStation(false);
          addColumn(oc);
        }
      }
    }
  }
}
