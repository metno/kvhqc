
#include "JumpToObservation.hh"

#include "common/EditAccess.hh"
#include "common/SingleObsBuffer.hh"
#include "common/SensorChooser.hh"
#include "common/HqcApplication.hh"

#include <qmessagebox.h>

#include "ui_jumptoobservation.h"

#define MILOGGER_CATEGORY "kvhqc.JumpToObservation"
#include "common/ObsLogging.hh"

JumpToObservation::JumpToObservation(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::JumpToObservation)
  , mDA(hqcApp->editAccess())
{
  ui->setupUi(this);

  mSensorChooser.reset(new SensorChooser(ui->textStation, ui->comboParam, ui->comboType, ui->comboLevel, ui->spinSensorNr, this));
  connect(mSensorChooser.get(), SIGNAL(valid(bool)), this, SLOT(slotValidSensor(bool)));

  ui->editObsTime->setDateTime(timeutil::nowWithMinutes0Seconds0());
}

JumpToObservation::~JumpToObservation()
{
}

void JumpToObservation::navigateTo(const SensorTime& st)
{
  mSensorChooser->setSensorTime(st);
  ui->editObsTime->setDateTime(timeutil::to_QDateTime(st.time));
}

void JumpToObservation::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    ui->retranslateUi(this);
  QDialog::changeEvent(event);
}

void JumpToObservation::accept()
{
  METLIBS_LOG_SCOPE();
  if (not mDA)
    return;

  const SensorTime st(selectedSensorTime());
  METLIBS_LOG_DEBUG(LOGVAL(st));
  if (not st.valid())
    return;

  SingleObsBuffer sob(st);
  sob.syncRequest(mDA);
  if (sob.get()) {
    QDialog::accept();
    Q_EMIT signalNavigateTo(st);
  } else {
    QMessageBox msg;
    msg.setWindowTitle(windowTitle());
    msg.setText(tr("No such observation found."));
    msg.setStandardButtons(QMessageBox::Retry);
    msg.setDefaultButton(QMessageBox::Retry);
    msg.exec();
  }
}

SensorTime JumpToObservation::selectedSensorTime() const
{
  const Sensor s = mSensorChooser->getSensor();
  if (s.stationId == -1 or s.paramId == -1 or s.typeId == -1)
    return SensorTime(); // invalid
  return SensorTime(s, timeutil::from_QDateTime(ui->editObsTime->dateTime()));
}

void JumpToObservation::slotValidSensor(bool valid)
{
  METLIBS_LOG_SCOPE(LOGVAL(valid));
  ui->editObsTime->setEnabled(valid);
  ui->buttonJump->setEnabled(valid);
}
