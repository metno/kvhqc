
#include "StationDataList.hh"

#include "common/ColumnFactory.hh"
#include "common/DataListModel.hh"
#include "common/KvHelpers.hh"
#include "common/ObsPgmRequest.hh"

#define MILOGGER_CATEGORY "kvhqc.StationDataList"
#include "common/ObsLogging.hh"

namespace {
const std::string VIEW_TYPE = "StationDataList";

const int WeatherParameters[] = {
  211,214,216,213,215,262,178,173,177,1,61,81,86,87,83,90,15,14,55,104,108,
  109,110,112,18,7,273,41,31,32,33,42,43,34,36,38,40,
  23,24,22,403,404,131,134,151,154,250,221,9,12
};
const size_t NWeatherParameters = sizeof(WeatherParameters)/sizeof(WeatherParameters[0]);
const int* WeatherParametersE = WeatherParameters + NWeatherParameters;
typedef std::set<int> int_s;
}

StationDataList::StationDataList(QWidget* parent)
  : TimespanDataList(parent)
  , mShowAggregated(true)
  , mObsPgmRequest(0)
{
  setWindowTitle(tr("Station Data"));
  setWindowIcon(QIcon("icons:weatherstation.svg"));
}

StationDataList::~StationDataList()
{
  delete mObsPgmRequest;
}

SensorTime StationDataList::sensorSwitch() const
{
  const SensorTime& sst = storeSensorTime(), cst = currentSensorTime();
  if (sst.sensor.stationId == cst.sensor.stationId and timeSpan().contains(cst.time))
    return sst;

  Sensor s = cst.sensor;
  if (s.typeId < 0)
    s.typeId = -s.typeId;
  s.paramId = 1;
  s.level = 0;
  s.sensor = 0;
  return SensorTime(s, cst.time);
}

void StationDataList::addSensorColumn(const Sensor& s, ObsColumn::Type type)
{
  if (DataColumn_p oc = ColumnFactory::columnForSensor(mDA, s, timeSpan(), type))
    model()->addColumn(oc);
}

void StationDataList::addSensorColumns(Sensor_s& alreadyShown, const Sensor& add)
{
  if (alreadyShown.insert(add).second) {
    addSensorColumn(add, ObsColumn::ORIGINAL);
    addSensorColumn(add, ObsColumn::NEW_CORRECTED);
  }
  
  if (not mShowAggregated)
    return;
  
  int_s aggregatedTo;
  Helpers::aggregatedParameters(add.paramId, aggregatedTo);
  Sensor agg(add);
  agg.typeId = -add.typeId;
  for (int_s::const_iterator it = aggregatedTo.begin(); it != aggregatedTo.end(); ++it) {
    agg.paramId = *it;
    if (alreadyShown.insert(agg).second)
      addSensorColumn(agg, ObsColumn::NEW_CORRECTED);
  }
}

void StationDataList::doSensorSwitch()
{
  METLIBS_LOG_TIME();
  setDefaultTimeSpan();

  hqc::int_s stationIds;
  stationIds.insert(storeSensorTime().sensor.stationId);
  delete mObsPgmRequest;
  mObsPgmRequest = new ObsPgmRequest(stationIds);
  connect(mObsPgmRequest, SIGNAL(complete()), this, SLOT(onObsPgmsComplete()));
  mObsPgmRequest->post();
}

void StationDataList::onObsPgmsComplete()
{
  METLIBS_LOG_TIME();

  // FIXME this relies on the implementation in TimespanDataList
  loadChanges();
  updateModel();
}

void StationDataList::updateModel()
{
  METLIBS_LOG_SCOPE(LOGVAL(currentSensorTime()) << LOGVAL(timeSpan()));

  Sensor s(currentSensorTime().sensor);
  if (s.typeId < 0)
    s.typeId = -s.typeId;
  const TimeSpan& time = timeSpan();

  Sensor_s columnSensors; // to avoid duplicate columns

  model()->removeAllColumns();
  model()->setTimeSpan(time);

  const hqc::kvObsPgm_v& opl = mObsPgmRequest->get(s.stationId);
  for(size_t i=0; i<NWeatherParameters; ++i) {
    const int paramId = WeatherParameters[i];
    for(hqc::kvObsPgm_v::const_iterator it = opl.begin(); it != opl.end(); ++it) {
      const kvalobs::kvObsPgm& op = *it;
      if (paramId != op.paramID())
        continue;
      if (s.typeId != op.typeID())
        continue;
      if (time.intersection(TimeSpan(op.fromtime(), op.totime())).undef())
        continue;

      addSensorColumns(columnSensors, Sensor(s.stationId, paramId, s.level, s.sensor, s.typeId));
      break;
    }
  }

  for(hqc::kvObsPgm_v::const_iterator it = opl.begin(); it != opl.end(); ++it) {
    const kvalobs::kvObsPgm& op = *it;
    if (s.typeId != op.typeID())
      continue;
    if (time.intersection(TimeSpan(op.fromtime(), op.totime())).undef())
      continue;
    if (std::find(WeatherParameters, WeatherParametersE, op.paramID()) != WeatherParametersE)
      continue;

    addSensorColumns(columnSensors, Sensor(s.stationId, op.paramID(), s.level, s.sensor, s.typeId));
  }
}

std::string StationDataList::viewType() const
{
  return VIEW_TYPE;
}
