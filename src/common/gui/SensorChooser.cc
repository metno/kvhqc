
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
#include "common/ObsLogging.hh"

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

  mParam->setModel(new ParamIdModel);
  mType ->setModel(new TypeIdModel);
}

SensorChooser::~SensorChooser()
{
  delete mParam->model();
  delete mType->model();
  // uninstall station completer?
}

void SensorChooser::setSensor(const Sensor& sensor)
{
  METLIBS_LOG_SCOPE(LOGVAL(sensor));

  if (sensor.valid()) {
    mStation->setText(QString::number(sensor.stationId));
    setParamId(sensor.paramId);
    setTypeId(sensor.typeId);
    setLevel(sensor.level);
    setSensorNr(sensor.sensor);
    onStationEdited("");
  }
}

void SensorChooser::setTimeRange(const TimeRange& time)
{
  METLIBS_LOG_SCOPE("not implemented");
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
  s.level = getLevel();
  s.sensor = getSensorNr();
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
  METLIBS_LOG_SCOPE();
  const int idx = mParam->currentIndex();
  METLIBS_LOG_DEBUG(LOGVAL(idx));
  if (idx < 0)
      return -1;
  return paramModel()->values().at(idx);
}

void SensorChooser::setParamId(int pid)
{
  METLIBS_LOG_SCOPE();
  const std::vector<int>& params = paramModel()->values();
  const std::vector<int>::const_iterator it = std::find(params.begin(), params.end(), pid);
  if (it != params.end()) {
    const int idx = it - params.begin();
    mParam->setCurrentIndex(idx);
    METLIBS_LOG_DEBUG(LOGVAL(idx));
  } else {
    mParam->setCurrentIndex(0);
  }
}

int SensorChooser::getTypeId() const
{
  METLIBS_LOG_SCOPE();
  const int idx = mType->currentIndex();
  METLIBS_LOG_DEBUG(LOGVAL(idx));
  if (idx < 0)
      return 0;
  return typeModel()->values().at(idx);
}

void SensorChooser::setTypeId(int tid)
{
  METLIBS_LOG_SCOPE();
  const std::vector<int>& types = typeModel()->values();
  const std::vector<int>::const_iterator it = std::find(types.begin(), types.end(), tid);
  if (it != types.end()) {
    const int idx = it - types.begin();
    mType->setCurrentIndex(idx);
    METLIBS_LOG_DEBUG(LOGVAL(idx));
  } else {
    mType->setCurrentIndex(0);
  }
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
    METLIBS_LOG_DEBUG(LOGVAL(stationParams.size()));
    goodStation &= (not stationParams.empty());
  }

  METLIBS_LOG_DEBUG(LOGVAL(goodStation));
  mParam->setEnabled(goodStation);

  const int paramId = getParamId();
  paramModel()->setValues(std::vector<int>(stationParams.begin(), stationParams.end()));
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
  METLIBS_LOG_DEBUG(LOGVAL(goodParam));
  mType->setEnabled(goodParam);

  const int typeId = getTypeId();
  typeModel()->setValues(std::vector<int>(stationTypes.begin(), stationTypes.end()));
  setTypeId(typeId);

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
  if (good) {
    const KvMetaDataBuffer::ObsPgmList& opgm = KvMetaDataBuffer::instance()->findObsPgm(stationId);
    BOOST_FOREACH(const kvalobs::kvObsPgm& op, opgm) {
      const int p = op.paramID(), t = op.typeID();
      if ((paramId == p and typeId == t)
          or (Helpers::aggregatedParameter(p, paramId) and typeId == -t))
      {
        levels.insert(op.level());
        METLIBS_LOG_DEBUG(LOGVAL(op.level()));
      }
    }
  }
  METLIBS_LOG_DEBUG(LOGVAL(good));
  mLevel->setEnabled(good);

  const int level = getLevel();
  setLevels(levels);
  setLevel(level);

  onLevelSelected(0);
}

void SensorChooser::onLevelSelected(int)
{
  METLIBS_LOG_SCOPE();
  const int stationId = getStationId();
  const int paramId   = getParamId();
  const int typeId    = getTypeId();
  const int level     = getLevel();
  METLIBS_LOG_DEBUG(LOGVAL(typeId));

  bool good = (stationId >= 60 and paramId >= 0 and typeId != 0);
  int maxSensor = 0;
  if (good) {
    const KvMetaDataBuffer::ObsPgmList& opgm = KvMetaDataBuffer::instance()->findObsPgm(stationId);
    BOOST_FOREACH(const kvalobs::kvObsPgm& op, opgm) {
      const int p = op.paramID(), t = op.typeID();
      if (level == op.level()
          and ((paramId == p and typeId == t) or (Helpers::aggregatedParameter(p, paramId) and typeId == -t)))
      {
        maxSensor = std::max(maxSensor, op.nr_sensor()-1);
      }
    }
  }
  METLIBS_LOG_DEBUG(LOGVAL(good));
  mSensorNr->setEnabled(good);

  const int sensorNr = getSensorNr();
  setMaxSensor(maxSensor);
  setSensorNr(sensorNr);

  Q_EMIT valid(good);
}

ParamIdModel* SensorChooser::paramModel() const
{
  return static_cast<ParamIdModel*>(mParam->model());
}

TypeIdModel* SensorChooser::typeModel() const
{
  return static_cast<TypeIdModel*>(mType->model());
}
