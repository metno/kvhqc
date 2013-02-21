
#include "RedistDialog.hh"

#include "AnalyseRR24.hh"
#include "RedistTableModel.hh"

#include "ui_watchrr_redist.h"

#define NDEBUG
#include "debug.hh"

RedistDialog::RedistDialog(QDialog* parent, EditAccessPtr da, const Sensor& sensor, const TimeRange& time, const TimeRange& editableTime)
  : QDialog(parent)
  , mDA(da)
  , mEditableTime(editableTime)
  , rtm(new RedistTableModel(da, sensor, time))
  , ui(new Ui::DialogRedist)
{
    LOG_SCOPE();
    ui->setupUi(this);
    connect(rtm.get(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(onDataChanged(const QModelIndex&, const QModelIndex&)));

    ui->table->setModel(rtm.get());
    ui->table->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->table->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->labelStationInfo->setText(tr("Station %1 [%2]").arg(sensor.stationId).arg(sensor.typeId));

    updateSumInfo();
}

RedistDialog::~RedistDialog()
{
    LOG_SCOPE();
}

void RedistDialog::updateSumInfo()
{
    const float cs = rtm->currentSum();
    ui->labelSum->setText(tr("Sum: %1mm").arg(cs, 0, 'f', 1));

    const float os = rtm->originalSum(), diff = cs - os;
    if( os >= 0 and fabs(diff) >= 0.05 )
        ui->labelSumWarn->setText(tr("Difference %1mm to original sum %2mm!").arg(diff, 0, 'f', 1).arg(os, 0, 'f', 1));
    else
        ui->labelSumWarn->setText("");
}

void RedistDialog::onDataChanged(const QModelIndex&, const QModelIndex&)
{
    updateSumInfo();
}

void RedistDialog::onButtonOk()
{
    LOG_SCOPE();
    RR24::redistribute(mDA, rtm->sensor(), rtm->time().t0(), mEditableTime, rtm->newCorrected());
    accept();
}
