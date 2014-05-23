
#include "StationDataList.hh"

#include "common/ColumnFactory.hh"
#include "common/DataListModel.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"

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
}

StationDataList::StationDataList(QWidget* parent)
  : TimespanDataList(parent)
{
  setWindowTitle(tr("Station Data"));
  setWindowIcon(QIcon("icons:weatherstation.svg"));
}

StationDataList::~StationDataList()
{
  // cannot be called from ~DynamicDataView as it calls virtual methods
  storeChanges();
}

SensorTime StationDataList::sensorSwitch() const
{
  const SensorTime& sst = storeSensorTime(), cst = currentSensorTime();
  if (sst.sensor.stationId == cst.sensor.stationId and timeSpan().contains(cst.time))
    return sst;

  Sensor s = cst.sensor;
  s.paramId = 1;
  s.level = 0;
  s.sensor = 0;
  return SensorTime(s, cst.time);
}

void StationDataList::addSensorColumns(DataListModel* model, const Sensor& s)
{
  DataColumn_p ocOrig = ColumnFactory::columnForSensor(mDA, s, timeSpan(), ObsColumn::ORIGINAL);
  if (ocOrig)
    model->addColumn(ocOrig);

  DataColumn_p ocCorr = ColumnFactory::columnForSensor(mDA, s, timeSpan(), ObsColumn::NEW_CORRECTED);
  if (ocCorr)
    model->addColumn(ocCorr);
}

DataListModel* StationDataList::makeModel()
{
  METLIBS_LOG_SCOPE(LOGVAL(currentSensorTime()) << LOGVAL(timeSpan()));

  const Sensor& s = currentSensorTime().sensor;
  const TimeSpan& time = timeSpan();

  std::auto_ptr<DataListModel> newModel(new DataListModel(mDA, time));

  const KvMetaDataBuffer::ObsPgmList& opl = KvMetaDataBuffer::instance()->findObsPgm(s.stationId);
  for(size_t i=0; i<NWeatherParameters; ++i) {
    const int paramId = WeatherParameters[i];
    for(KvMetaDataBuffer::ObsPgmList::const_iterator it = opl.begin(); it != opl.end(); ++it) {
      const kvalobs::kvObsPgm& op = *it;
      if (paramId != op.paramID())
        continue;
      if (s.typeId != op.typeID())
        continue;
      if (time.intersection(TimeSpan(op.fromtime(), op.totime())).undef())
        continue;

      addSensorColumns(newModel.get(), Sensor(s.stationId, paramId, s.level, s.sensor, s.typeId));
      break;
    }
  }

  for(KvMetaDataBuffer::ObsPgmList::const_iterator it = opl.begin(); it != opl.end(); ++it) {
    const kvalobs::kvObsPgm& op = *it;
    if (s.typeId != op.typeID())
      continue;
    if (time.intersection(TimeSpan(op.fromtime(), op.totime())).undef())
      continue;
    if (std::find(WeatherParameters, WeatherParametersE, op.paramID()) != WeatherParametersE)
      continue;
    
    addSensorColumns(newModel.get(), Sensor(s.stationId, op.paramID(), s.level, s.sensor, s.typeId));
  }

  return newModel.release();
}

std::string StationDataList::viewType() const
{
  return VIEW_TYPE;
}
