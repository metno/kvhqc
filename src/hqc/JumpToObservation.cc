
#include "JumpToObservation.hh"

#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/ParamIdModel.hh"
#include "common/StationIdCompletion.hh"
#include "common/TypeIdModel.hh"

#include <QtGui/qmessagebox.h>
#include <QtGui/QStringListModel>
#include <QtGui/QCompleter>
#include <QtGui/QValidator>

#include <boost/foreach.hpp>

#include "ui_jumptoobservation.h"

#define MILOGGER_CATEGORY "kvhqc.JumpToObservation"
#include "common/ObsLogging.hh"

JumpToObservation::JumpToObservation(ObsAccessPtr da, QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::JumpToObservation)
  , mDA(da)
{
  ui->setupUi(this);

  Helpers::installStationIdCompleter(this, ui->textStation);

  std::vector<int> empty;
  ui->comboParam->setModel(new ParamIdModel(empty));
  ui->comboType ->setModel(new TypeIdModel(empty));
  
  ui->editObsTime->setDateTime(timeutil::nowWithMinutes0Seconds0());
}

JumpToObservation::~JumpToObservation()
{
  delete ui->comboParam->model();
  delete ui->comboType->model();
}

void JumpToObservation::accept()
{
  METLIBS_LOG_SCOPE();
  if (mDA) {
    const SensorTime st(selectedSensorTime());
    METLIBS_LOG_DEBUG(LOGVAL(st));
    const ObsSubscription sub(st.sensor.stationId, TimeRange(st.time, st.time));
    mDA->addSubscription(sub);
    if (mDA->find(st)) {
      QDialog::accept();
      /*emit*/ signalNavigateTo(st);
    } else {
      QMessageBox msg;
      msg.setWindowTitle(windowTitle());
      msg.setText(tr("No such observation found."));
      msg.setStandardButtons(QMessageBox::Retry);
      msg.setDefaultButton(QMessageBox::Retry);
      msg.exec();
    }
    mDA->removeSubscription(sub);
  }
}

SensorTime JumpToObservation::selectedSensorTime() const
{
  Sensor s;
  s.stationId = getStationId();
  s.paramId  = getParamId();
  s.typeId   = getTypeId();
  s.level    = getLevel();
  s.sensor   = getSensorNr();

  if (s.stationId == -1 or s.paramId == -1 or s.typeId == -1)
    return SensorTime(); // invalid

  return SensorTime(s, timeutil::from_QDateTime(ui->editObsTime->dateTime()));
}

int JumpToObservation::getStationId() const
{
  bool ok;
  const int stationId = ui->textStation->text().toInt(&ok);
  if (not ok)
    return -1;
  return stationId;
}

int JumpToObservation::getSensorNr() const
{
  return ui->spinSensor->value();
}

int JumpToObservation::getLevel() const
{
  return ui->spinLevel->value();
}

int JumpToObservation::getParamId() const
{
  const int idx = ui->comboParam->currentIndex();
  if (idx < 0)
      return -1;
  ParamIdModel* pim = static_cast<ParamIdModel*>(ui->comboParam->model());
  return pim->parameterIds().at(idx);
}

int JumpToObservation::getTypeId() const
{
  const int idx = ui->comboType->currentIndex();
  if (idx < 0)
      return -1;
  TypeIdModel* tim = static_cast<TypeIdModel*>(ui->comboType->model());
  return tim->typeIds().at(idx);
}

void JumpToObservation::onStationEdited()
{
  METLIBS_LOG_SCOPE();
  std::set<int> stationParams;

  bool goodStation = false;
  const int stationId = ui->textStation->text().toInt(&goodStation);
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

  ui->comboParam->setEnabled(goodStation);

  delete ui->comboParam->model();
  ui->comboParam->setModel(new ParamIdModel(std::vector<int>(stationParams.begin(), stationParams.end())));
  if (goodStation)
    ui->comboParam->setCurrentIndex(0);
  onParameterSelected(0);
}

void JumpToObservation::onParameterSelected(int)
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

  delete ui->comboType->model();
  ui->comboType->setModel(new TypeIdModel(std::vector<int>(stationTypes.begin(), stationTypes.end())));
  if (goodParam)
    ui->comboType->setCurrentIndex(0);

  ui->comboType->setEnabled(goodParam);
  ui->buttonJump->setEnabled(goodParam);
}
