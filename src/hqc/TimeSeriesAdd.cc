
#include "TimeSeriesAdd.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/ParamIdModel.hh"
#include "common/StationIdCompletion.hh"
#include "common/TypeIdModel.hh"

#include <QtGui/QStringListModel>
#include <QtGui/QCompleter>
#include <QtGui/QValidator>

#include <boost/foreach.hpp>

#include "ui_ts_add.h"

#define MILOGGER_CATEGORY "kvhqc.TimeSeriesAdd"
#include "util/HqcLogging.hh"

TimeSeriesAdd::TimeSeriesAdd(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::TimeSeriesAdd)
{
  ui->setupUi(this);

  Helpers::installStationIdCompleter(this, ui->textStation);

  std::vector<int> empty;
  ui->comboParam->setModel(new ParamIdModel(empty));
  ui->comboParam->setModel(new TypeIdModel(empty));
}

TimeSeriesAdd::~TimeSeriesAdd()
{
  delete ui->comboParam->model();
  delete ui->comboType->model();
}

Sensor TimeSeriesAdd::selectedSensor() const
{
  Sensor s;
  s.stationId = getStationId();
  s.paramId = getParamId();
  s.typeId = getTypeId();

  if (s.stationId == -1 or s.paramId == -1 or s.typeId == -1)
    return Sensor(); // invalid sensor

  return s;
}

int TimeSeriesAdd::getStationId() const
{
  bool ok;
  const int stationId = ui->textStation->text().toInt(&ok);
  if (not ok)
    return -1;
  return stationId;
}

int TimeSeriesAdd::getParamId() const
{
  const int idx = ui->comboParam->currentIndex();
  if (idx < 0)
      return -1;
  ParamIdModel* pim = static_cast<ParamIdModel*>(ui->comboParam->model());
  return pim->values().at(idx);
}

int TimeSeriesAdd::getTypeId() const
{
  const int idx = ui->comboType->currentIndex();
  if (idx < 0)
      return -1;
  TypeIdModel* tim = static_cast<TypeIdModel*>(ui->comboType->model());
  return tim->values().at(idx);
}

void TimeSeriesAdd::onStationEdited()
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

  ui->comboParam    ->setEnabled(goodStation);

  delete ui->comboParam->model();
  ui->comboParam->setModel(new ParamIdModel(std::vector<int>(stationParams.begin(), stationParams.end())));
  if (goodStation)
    ui->comboParam->setCurrentIndex(0);
  onParameterSelected(0);
}

void TimeSeriesAdd::onParameterSelected(int)
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
  ui->buttonOk->setEnabled(goodParam);
}
