
#include "DataListAddColumn.hh"

#include "KvMetaDataBuffer.hh"
#include "ParamIdModel.hh"
#include "StationIdModel.hh"
#include "TypeIdModel.hh"

#include <QtGui/QStringListModel>
#include <QtGui/QCompleter>
#include <QtGui/QValidator>

#include <boost/foreach.hpp>

#include "ui_dl_addcolumn.h"

#define MILOGGER_CATEGORY "kvhqc.DataListAddColumn"
#include "HqcLogging.hh"

DataListAddColumn::DataListAddColumn(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::DataListAddColumn)
{
  ui->setupUi(this);

  QCompleter *completer = new QCompleter(this);

  QTableView* completionPopup = new QTableView(this);
  completionPopup->horizontalHeader()->setVisible(false);
  completionPopup->verticalHeader()->setVisible(false);
  completionPopup->verticalHeader()->setDefaultSectionSize(20);
  completionPopup->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  completer->setPopup(completionPopup);

  StationIdModel* cmodel = new StationIdModel(completer);
  completer->setModel(cmodel);

  completer->setCompletionColumn(0);
  completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  ui->textStation->setCompleter(completer);

  QValidator *validator = new QIntValidator(cmodel->minStationId(), cmodel->maxStationId(), this);
  ui->textStation->setValidator(validator);

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
  s.stationId = getStationId();
  s.paramId = getParamId();
  s.typeId = getTypeId();

  if (s.stationId == -1 or s.paramId == -1 or s.typeId == -1)
    return Sensor(); // invalid sensor

  return s;
}

int DataListAddColumn::getStationId() const
{
  bool ok;
  const int stationId = ui->textStation->text().toInt(&ok);
  if (not ok)
    return -1;
  return stationId;
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
  METLIBS_LOG_SCOPE();
  std::set<int> stationParams;

  bool goodStation = false;
  const int stationId = ui->textStation->text().toInt(&goodStation);
  if (goodStation)
    goodStation &= KvMetaDataBuffer::instance()->isKnownStation(stationId);

  if (goodStation) {
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
    goodStation &= (not stationParams.empty());
  }

  ui->radioCorrected->setEnabled(goodStation);
  ui->radioOriginal ->setEnabled(goodStation);
  ui->radioFlags    ->setEnabled(goodStation);
  ui->radioModel    ->setEnabled(goodStation);
  ui->comboParam    ->setEnabled(goodStation);

  delete ui->comboParam->model();
  ui->comboParam->setModel(new ParamIdModel(std::vector<int>(stationParams.begin(), stationParams.end())));
  if (goodStation)
    ui->comboParam->setCurrentIndex(0);
  onParameterSelected(0);
}

void DataListAddColumn::onParameterSelected(int)
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
