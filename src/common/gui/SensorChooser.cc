
#include "SensorChooser.hh"

#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/ParamIdModel.hh"
#include "common/StationIdCompletion.hh"
#include "common/TypeIdModel.hh"

#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>

#include <boost/foreach.hpp>

#include <vector>

#define MILOGGER_CATEGORY "kvhqc.SensorChooser"
#include "util/HqcLogging.hh"

SensorChooser::SensorChooser(QLineEdit* station, QComboBox* param, QComboBox* type, QComboBox* level, QSpinBox* sensorNr, QObject* parent)
  : QObject(parent)
  , mStation(station)
  , mParam(param)
  , mType(type)
  , mLevel(level)
  , mSensorNr(sensorNr)
{
  METLIBS_LOG_SCOPE();
  Helpers::installStationIdCompleter(this, mStation);
  QObject::connect(mStation, SIGNAL(textChanged(QString)),     this, SLOT(onStationEdited(QString)));
  QObject::connect(mParam,   SIGNAL(currentIndexChanged(int)), this, SLOT(onParameterSelected(int)));
  QObject::connect(mType,    SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeSelected(int)));
  QObject::connect(mLevel,   SIGNAL(currentIndexChanged(int)), this, SLOT(onLevelSelected(int)));

  std::vector<int> empty;
  mParam->setModel(new ParamIdModel(empty));
  mType ->setModel(new TypeIdModel(empty));
}

SensorChooser::~SensorChooser()
{
  delete mParam->model();
  delete mType->model();
  // uninstall station completer?
}

void SensorChooser::setSensor(const Sensor& sensor)
{
  METLIBS_LOG_SCOPE();
  mStation->setText(QString::number(sensor.stationId));
  
  setParamId(sensor.paramId);
  setTypeId(sensor.typeId);
  setLevel(sensor.level);
  setSensorNr(sensor.sensor);
}

void SensorChooser::setTimeRange(const TimeRange& time)
{
  METLIBS_LOG_SCOPE();
}

bool SensorChooser::isValid()
{
  return getSensor().valid();
}

Sensor SensorChooser::getSensor()
{
  Sensor s;
  s.stationId = getStationId();
  s.paramId = getParamId();
  s.typeId = getTypeId();
  s.sensor = getSensorNr();
  s.level = getLevel();

  if (s.stationId == -1 or s.paramId == -1 or s.typeId == 0)
    return Sensor(); // invalid sensor

  return s;
}

int SensorChooser::getStationId() const
{
  bool ok;
  const int stationId = mStation->text().toInt(&ok);
  if (not ok)
    return -1;
  return stationId;
}

int SensorChooser::getParamId() const
{
  const int idx = mParam->currentIndex();
  if (idx < 0)
      return -1;
  ParamIdModel* pim = static_cast<ParamIdModel*>(mParam->model());
  return pim->values().at(idx);
}

void SensorChooser::setParamId(int pid)
{
  const std::vector<int> params = static_cast<ParamIdModel*>(mParam->model())->values();
  const std::vector<int>::const_iterator it = std::find(params.begin(), params.end(), pid);
  if (it != params.end())
    mParam->setCurrentIndex(it - params.begin());
}

int SensorChooser::getTypeId() const
{
  const int idx = mType->currentIndex();
  if (idx < 0)
      return 0;
  TypeIdModel* tim = static_cast<TypeIdModel*>(mType->model());
  return tim->values().at(idx);
}

void SensorChooser::setTypeId(int tid)
{
  const std::vector<int> types = static_cast<TypeIdModel*>(mType->model())->values();
  const std::vector<int>::const_iterator it = std::find(types.begin(), types.end(), tid);
  if (it != types.end())
    mType->setCurrentIndex(it - types.begin());
}

int SensorChooser::getSensorNr() const
{
  return mSensorNr->value();
}

void SensorChooser::setSensorNr(int s)
{
  if (s >= mSensorNr->minimum() and s <= mSensorNr->maximum())
    mSensorNr->setValue(s);
}

int SensorChooser::getLevel() const
{
  return mLevel->currentText().toInt();
}

void SensorChooser::setLevel(int level)
{
  QString ltext = QString::number(level);
  for (int lidx = 0; lidx < mLevel->count(); ++lidx) {
    if (ltext == mLevel->itemText(lidx)) {
      mLevel->setCurrentIndex(lidx);
      break;
    }
  }
}

void SensorChooser::setLevels(const std::set<int>& levels)
{
  mLevel->clear();
  if (levels.find(0) == levels.end())
    mLevel->addItem("0");
  BOOST_FOREACH(int l, levels)
      mLevel->addItem(QString::number(l));
  mLevel->setCurrentText("0");
}

void SensorChooser::setMaxSensor(int maxSensor)
{
  mSensorNr->setMaximum(maxSensor);
  mSensorNr->setValue(0);
}

void SensorChooser::onStationEdited(const QString&)
{
  METLIBS_LOG_SCOPE();
  std::set<int> stationParams;

  bool goodStation = false;
  const int stationId = mStation->text().toInt(&goodStation);
  if (goodStation)
    goodStation &= KvMetaDataBuffer::instance()->isKnownStation(stationId);

  if (goodStation) {
    const KvMetaDataBuffer::ObsPgmList& opgm = KvMetaDataBuffer::instance()->findObsPgm(stationId);
    BOOST_FOREACH(const kvalobs::kvObsPgm& op, opgm) {
      const int p = op.paramID();
      if (p == kvalobs::PARAMID_V4S or p == kvalobs::PARAMID_V5S or p == kvalobs::PARAMID_V6S)
        continue;
      stationParams.insert(p);
      Helpers::aggregatedParameters(p, stationParams);
    }
    goodStation &= (not stationParams.empty());
  }

  mParam->setEnabled(goodStation);

  const int paramId = getParamId();
  delete mParam->model();
  mParam->setModel(new ParamIdModel(std::vector<int>(stationParams.begin(), stationParams.end())));
  if (goodStation)
    mParam->setCurrentIndex(0);
  setParamId(paramId);
  onParameterSelected(0);
}

void SensorChooser::onParameterSelected(int)
{
  METLIBS_LOG_SCOPE();
  const int stationId = getStationId();
  const int paramId   = getParamId();

  bool goodParam = paramId >= 0;
  std::set<int> stationTypes;
  if (goodParam) {
    const KvMetaDataBuffer::ObsPgmList& opgm = KvMetaDataBuffer::instance()->findObsPgm(stationId);
    BOOST_FOREACH(const kvalobs::kvObsPgm& op, opgm) {
      const int p = op.paramID();
      if (p == paramId)
        stationTypes.insert(op.typeID());
      if (Helpers::aggregatedParameter(p, paramId))
        stationTypes.insert(-op.typeID());
    }
    goodParam &= (not stationTypes.empty());
  }

  const int typeId = getTypeId();
  delete mType->model();
  mType->setModel(new TypeIdModel(std::vector<int>(stationTypes.begin(), stationTypes.end())));
  if (goodParam)
    mType->setCurrentIndex(0);
  setTypeId(typeId);
  mType->setEnabled(goodParam);
  onTypeSelected(0);
}

void SensorChooser::onTypeSelected(int)
{
  METLIBS_LOG_SCOPE();
  const int stationId = getStationId();
  const int paramId   = getParamId();
  const int typeId    = getTypeId();
  METLIBS_LOG_DEBUG(LOGVAL(typeId));

  bool good = (stationId >= 60 and paramId >= 0 and typeId != 0);
  std::set<int> levels;
  int maxSensor = 0;
  if (good) {
    const KvMetaDataBuffer::ObsPgmList& opgm = KvMetaDataBuffer::instance()->findObsPgm(stationId);
    BOOST_FOREACH(const kvalobs::kvObsPgm& op, opgm) {
      const int p = op.paramID(), t = op.typeID();
      if ((paramId == p and typeId == t)
          or (Helpers::aggregatedParameter(p, paramId) and typeId == -t))
      {
        levels.insert(op.level());
        maxSensor = std::max(maxSensor, op.nr_sensor()-1);
        METLIBS_LOG_DEBUG(LOGVAL(op.level()) << LOGVAL(op.nr_sensor()));
      }
    }
  }
  const int level = getLevel(), sensorNr = getSensorNr();
  setLevels(levels);
  setMaxSensor(maxSensor);
  setLevel(level);
  setSensorNr(sensorNr);

  mLevel->setEnabled(good);
  mSensorNr->setEnabled(good);
  /*emit*/ valid(good);
}

void SensorChooser::onLevelSelected(int)
{
  METLIBS_LOG_SCOPE();
}
