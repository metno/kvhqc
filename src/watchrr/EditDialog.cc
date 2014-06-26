
#include "EditDialog.hh"

#include "EditTableModel.hh"

#include "AnalyseRR24.hh"

#include "ui_watchrr_edit.h"

#define MILOGGER_CATEGORY "kvhqc.EditDialog"
#include "util/HqcLogging.hh"

EditDialog::EditDialog(QDialog* parent, TaskAccess_p da, const Sensor& sensor, const TimeSpan& time, const TimeSpan& editableTime)
  : QDialog(parent)
  , mDA(da)
  , mEditableTime(editableTime)
  , mSensor(sensor)
  , etm(new EditTableModel(da, sensor, time))
  , ui(new Ui::DialogEdit)
{
  ui->setupUi(this);
  ui->table->setModel(etm.get());
  ui->table->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  ui->table->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  ui->labelStationInfo->setText(tr("Station %1 [%2]").arg(sensor.stationId).arg(sensor.typeId));
}

EditDialog::~EditDialog()
{
}

void EditDialog::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange) {
    ui->retranslateUi(this);
    ui->labelStationInfo->setText(tr("Station %1 [%2]").arg(mSensor.stationId).arg(mSensor.typeId));
  }
  QDialog::changeEvent(event);
}

void EditDialog::onAcceptAll()
{
  etm->acceptAll();
}

void EditDialog::onRejectAll()
{
  etm->rejectAll();
}

void EditDialog::onButtonOk()
{
  METLIBS_LOG_SCOPE();
  RR24::singles(mDA, etm->sensor(), etm->time().t0(), mEditableTime, etm->newCorrected(), etm->acceptReject());
  accept();
}
