
#include "RedistDialog.hh"

#include "RedistTableModel.hh"
#include "AnalyseRR24.hh"

#include "common/KvHelpers.hh"
#include "common/ObsPgmRequest.hh"

#include "ui_watchrr_redist.h"

#define MILOGGER_CATEGORY "kvhqc.RedistDialog"
#include "util/HqcLogging.hh"

RedistDialog::RedistDialog(QDialog* parent, TaskAccess_p da, const Sensor& sensor, const TimeSpan& time, const TimeSpan& editableTime)
  : QDialog(parent)
  , mDA(da)
  , mEditableTime(editableTime)
  , mSensor(sensor)
  , rtm(new RedistTableModel(da, sensor, time))
  , ui(new Ui::DialogRedist)
{
  METLIBS_LOG_SCOPE();
  ui->setupUi(this);
  connect(rtm.get(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
      this, SLOT(onDataChanged(const QModelIndex&, const QModelIndex&)));

  ui->table->setModel(rtm.get());
  ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  ui->table->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

  updateSumInfo();
}

RedistDialog::~RedistDialog()
{
  METLIBS_LOG_SCOPE();
}

void RedistDialog::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange) {
    ui->retranslateUi(this);
    updateSumInfo();
  }
  QDialog::changeEvent(event);
}

void RedistDialog::updateSumInfo()
{
  ui->labelStationInfo->setText(tr("Station %1 [%2]").arg(mSensor.stationId).arg(mSensor.typeId));

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
  METLIBS_LOG_SCOPE();
  RR24::redistribute(mDA, rtm->sensor(), rtm->time().t0(), mEditableTime, rtm->newCorrected());
  accept();
}

void RedistDialog::onButtonAuto()
{
  METLIBS_LOG_SCOPE();

  hqc::int_s stationIds = KvMetaDataBuffer::instance()->findNeighborStationIds(rtm->sensor().stationId);
  stationIds.insert(rtm->sensor().stationId);

  std::unique_ptr<ObsPgmRequest> op(new ObsPgmRequest(stationIds));
  op->sync();
  
  std::vector<float> values = rtm->newCorrected();
  if (RR24::redistributeProposal(mDA, rtm->sensor(), rtm->time(), op.get(), values))
    rtm->setNewCorrected(values);
}
