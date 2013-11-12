
#include "TimeSeriesAdd.hh"

#include "common/gui/SensorChooser.hh"

#include "ui_ts_add.h"

#define MILOGGER_CATEGORY "kvhqc.TimeSeriesAdd"
#include "util/HqcLogging.hh"

TimeSeriesAdd::TimeSeriesAdd(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::TimeSeriesAdd)
{
  ui->setupUi(this);

  mSensorChooser.reset(new SensorChooser(ui->textStation, ui->comboParam, ui->comboType, ui->comboLevel, ui->spinSensorNr, this));
  connect(mSensorChooser.get(), SIGNAL(valid(bool)), this, SLOT(slotValidSensor(bool)));
}

TimeSeriesAdd::~TimeSeriesAdd()
{
}

Sensor TimeSeriesAdd::selectedSensor() const
{
  return mSensorChooser->getSensor();
}

void TimeSeriesAdd::slotValidSensor(bool valid)
{
  ui->buttonOk->setEnabled(valid);
}
