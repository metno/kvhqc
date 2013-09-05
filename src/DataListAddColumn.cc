
#include "DataListAddColumn.hh"

#include "KvMetaDataBuffer.hh"
#include "ParamIdModel.hh"
#include "TypeIdModel.hh"

#include <boost/foreach.hpp>

#include "ui_dl_addcolumn.h"

#define MILOGGER_CATEGORY "kvhqc.DataListAddColumn"
#include "HqcLogging.hh"

DataListAddColumn::DataListAddColumn(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::DataListAddColumn)
{
  ui->setupUi(this);
  std::vector<int> empty;
  ui->comboParam->setModel(new ParamIdModel(empty));
  ui->comboParam->setModel(new TypeIdModel(empty));
}

DataListAddColumn::~DataListAddColumn()
{
  delete ui->comboParam->model();
  delete ui->comboType->model();
}

Sensor DataListAddColumn::selectedSensor() const
{
  Sensor s;

  bool ok;
  s.stationId = ui->textStation->text().toInt(&ok);
  if (not ok)
    return s;
  s.paramId = getParamId();
  s.typeId = getTypeId();
  return s;
}

DataList::ColumnType DataListAddColumn::selectedColumnType() const
{
  DataList::ColumnType ct = DataList::CORRECTED;
  if (ui->radioOriginal->isChecked())
    ct = DataList::ORIGINAL;
  else if (ui->radioFlags->isChecked())
    ct = DataList::FLAGS;
  if (ui->radioModel->isChecked())
    ct = DataList::MODEL;
  return ct;
}

int DataListAddColumn::selectedTimeOffset() const
{
  return ui->spinTimeOffset->value();
}

void DataListAddColumn::onStationEdited()
{
  std::set<int> stationParams;

  bool ok = true;
  const int stationId = ui->textStation->text().toInt(&ok);
  if (ok and KvMetaDataBuffer::instance()->isKnownStation(stationId)) {
    const KvMetaDataBuffer::ObsPgmList& opgm = KvMetaDataBuffer::instance()->findObsPgm(stationId);
    bool haveRRacc = false;
    BOOST_FOREACH(const kvalobs::kvObsPgm& op, opgm) {
      const int p = op.paramID();
      stationParams.insert(p);
      if (p >= kvalobs::PARAMID_RR_01 and p < kvalobs::PARAMID_RR_24)
        haveRRacc = true;
    }
    if (haveRRacc) {
      for (int p=kvalobs::PARAMID_RR_01; p<=kvalobs::PARAMID_RR_24; ++p)
        stationParams.insert(p);
    }
  }
  const std::vector<int> paramIds(stationParams.begin(), stationParams.end());
  delete ui->comboParam->model();
  ui->comboParam->setModel(new ParamIdModel(paramIds));

  const bool enable = (not stationParams.empty());
  ui->comboParam->setEnabled(enable);
  if (enable)
    ui->comboParam->setCurrentIndex(0);
  onParameterSelected(0);
}

int DataListAddColumn::getParamId() const
{
  const int idx = ui->comboParam->currentIndex();
  if (idx < 0)
      return -1;
  ParamIdModel* pim = static_cast<ParamIdModel*>(ui->comboParam->model());
  return pim->parameterIds().at(idx);
}

int DataListAddColumn::getTypeId() const
{
  const int idx = ui->comboType->currentIndex();
  if (idx < 0)
      return -1;
  TypeIdModel* tim = static_cast<TypeIdModel*>(ui->comboType->model());
  return tim->typeIds().at(idx);
}

void DataListAddColumn::onParameterSelected(int)
{
  const int stationId = ui->textStation->text().toInt();
  const int paramId = getParamId();
  
  std::set<int> stationTypes;
  if (paramId >= 0) {
    const KvMetaDataBuffer::ObsPgmList& opgm = KvMetaDataBuffer::instance()->findObsPgm(stationId);
    BOOST_FOREACH(const kvalobs::kvObsPgm& op, opgm) {
      const int p = op.paramID();
      if (p == paramId) {
        stationTypes.insert(op.typeID());
        if (p > kvalobs::PARAMID_RR_01 and p <= kvalobs::PARAMID_RR_24)
          stationTypes.insert(-op.typeID());
      }
    }
  }

  const std::vector<int> typeIds(stationTypes.begin(), stationTypes.end());
  delete ui->comboType->model();
  ui->comboType->setModel(new TypeIdModel(typeIds));
    ui->comboType->setCurrentIndex(0);

  const bool enable = (not stationTypes.empty());
  ui->comboType->setEnabled(enable);
  if (enable)
    ui->comboType->setCurrentIndex(0);
  ui->buttonOk->setEnabled(enable);
}
