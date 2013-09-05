
#include "EditDialog.hh"

#include "AnalyseRR24.hh"
#include "EditTableModel.hh"

#include "ui_watchrr_edit.h"

#define MILOGGER_CATEGORY "kvhqc.EditDialog"
#include "HqcLogging.hh"

EditDialog::EditDialog(QDialog* parent, EditAccessPtr da, const Sensor& sensor, const TimeRange& time, const TimeRange& editableTime)
  : QDialog(parent)
  , mDA(da)
  , mEditableTime(editableTime)
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
