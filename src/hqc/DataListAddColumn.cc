
#include "DataListAddColumn.hh"

#include "common/SensorChooser.hh"

#include "ui_dl_addcolumn.h"

#define MILOGGER_CATEGORY "kvhqc.DataListAddColumn"
#include "util/HqcLogging.hh"

DataListAddColumn::DataListAddColumn(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::DataListAddColumn)
{
  ui->setupUi(this);
  mSensorChooser.reset(new SensorChooser(ui->textStation, ui->comboParam, ui->comboType, ui->comboLevel, ui->spinSensorNumber, this));
  connect(mSensorChooser.get(), SIGNAL(valid(bool)), this, SLOT(slotValidSensor(bool)));
}

DataListAddColumn::~DataListAddColumn()
{
  delete ui->comboParam->model();
  delete ui->comboType->model();
}

void DataListAddColumn::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    ui->retranslateUi(this);
  QDialog::changeEvent(event);
}

void DataListAddColumn::setSensor(const Sensor& sensor)
{
  mSensorChooser->setSensor(sensor);
}

Sensor DataListAddColumn::selectedSensor() const
{
  return mSensorChooser->getSensor();
}

AutoDataList::ColumnType DataListAddColumn::selectedColumnType() const
{
  AutoDataList::ColumnType ct = AutoDataList::CORRECTED;
  if (ui->radioOriginal->isChecked())
    ct = AutoDataList::ORIGINAL;
  else if (ui->radioFlags->isChecked())
    ct = AutoDataList::FLAGS;
  if (ui->radioModel->isChecked())
    ct = AutoDataList::MODEL;
  return ct;
}

int DataListAddColumn::selectedTimeOffset() const
{
  return ui->spinTimeOffset->value();
}

void DataListAddColumn::slotValidSensor(bool valid)
{
  ui->radioCorrected->setEnabled(valid);
  ui->radioOriginal ->setEnabled(valid);
  ui->radioFlags    ->setEnabled(valid);
  ui->radioModel    ->setEnabled(valid);
  ui->buttonOk->setEnabled(valid);
}
