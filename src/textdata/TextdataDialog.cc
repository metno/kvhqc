
#include "TextdataDialog.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/StationIdCompletion.hh"
#include "util/MiDateTimeEdit.hh"
#include "util/timeutil.hh"

#include <QEvent>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

TextDataDialog::TextDataDialog(const SensorTime& st, QWidget* parent)
    : QDialog(parent)
{
  setWindowIcon(QIcon("icons:textdata.svg"));

  textLabel0 = new QLabel(this);
  textLabel1 = new QLabel(this);
  textLabel2 = new QLabel(this);
  textLabel3 = new QLabel(this);
  stationEdit = new QLineEdit(this);

  Helpers::installStationIdCompleter(this, stationEdit);

  QDateTime ldtto;
  if (st.valid()) {
    stnr = st.sensor.stationId;
    stationEdit->setText(QString::number(stnr));
    ldtto = timeutil::to_QDateTime(st.time);
  } else {
    stnr = 0;
    ldtto = timeutil::nowWithMinutes0Seconds0();
  }
  dtto = ldtto;
  dtfrom = dtto.addDays(-2);
  fromEdit = new MiDateTimeEdit(dtfrom,this);
  toEdit   = new MiDateTimeEdit(dtto,this);
  fromEdit->setMaximumDate(dtto.date());
  fromEdit->setMaximumTime(dtto.time());
  fromEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
  toEdit->setMinimumDate(dtfrom.date());
  toEdit->setMinimumTime(dtfrom.time());
  toEdit->setDisplayFormat("yyyy-MM-dd hh:mm");

  connect(stationEdit, SIGNAL(textChanged(const QString&)),
      this, SLOT(setStation(const QString&)));

  connect(fromEdit, SIGNAL(dateTimeChanged(const QDateTime&)),
      this, SLOT(setFromTime(const QDateTime&)));

  connect(toEdit, SIGNAL(dateTimeChanged(const QDateTime&)),
      this, SLOT(setToTime(const QDateTime&)));

  okButton = new QPushButton(this);
  okButton->setGeometry(20, 620, 90, 30);
  okButton->setFont(QFont("Arial", 9));

  cancelButton = new QPushButton(this);
  cancelButton->setGeometry(120, 620, 90, 30);
  cancelButton->setFont(QFont("Arial", 9));

  QVBoxLayout* topLayout = new QVBoxLayout(this);
  QHBoxLayout* stationLayout = new QHBoxLayout();
  QHBoxLayout* fromLayout = new QHBoxLayout();
  QHBoxLayout* toLayout = new QHBoxLayout();
  QHBoxLayout* buttonLayout = new QHBoxLayout();

  stationLayout->addWidget(textLabel0);
  stationLayout->addWidget(stationEdit);
  fromLayout->addWidget(textLabel2);
  fromLayout->addWidget(fromEdit);
  toLayout->addWidget(textLabel3);
  toLayout->addWidget(toEdit);
  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(cancelButton);

  topLayout->addLayout(stationLayout);
  topLayout->addWidget(textLabel1);
  topLayout->addLayout(fromLayout);
  topLayout->addLayout(toLayout);
  topLayout->addLayout(buttonLayout);

  retranslateUi();

  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(okButton,     SIGNAL(clicked()), this, SLOT(accept()));
}

void TextDataDialog::retranslateUi()
{
  setWindowTitle(tr("TextData"));
  textLabel0->setText(tr("Station"));
  textLabel1->setText(tr("Time range"));
  textLabel2->setText(tr("From"));
  textLabel3->setText(tr("To"));
  okButton->setText(tr("OK"));
  cancelButton->setText(tr("Cancel"));
}

void TextDataDialog::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QDialog::changeEvent(event);
}

void TextDataDialog::setStation(const QString& st) {
  bool ok;
  stnr = st.toInt(&ok);
}

void TextDataDialog::setFromTime(const QDateTime& dt) {
  dtfrom = dt;
}

void TextDataDialog::setToTime(const QDateTime& dt) {
  dtto = dt;
}

TimeSpan TextDataDialog::getTimeSpan() const
{
  const timeutil::ptime f = timeutil::from_QDateTime(timeutil::clearedMinutesAndSeconds(dtfrom));
  const timeutil::ptime t = timeutil::from_QDateTime(timeutil::clearedMinutesAndSeconds(dtto));
  return TimeSpan(f, t);
}

void TextDataDialog::accept()
{
  if (KvMetaDataBuffer::instance()->isKnownStation(stnr)) {
    QDialog::accept();
  } else {
    QMessageBox::information( this, tr("TextData"),
        tr("Illegal station number. Choose a different station number."));
  }
}
