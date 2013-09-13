
#include "TextdataDialog.hh"

#include "common/KvMetaDataBuffer.hh"
#include "util/gui/MiDateTimeEdit.hh"
#include "util/timeutil.hh"

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/qmessagebox.h>
#include <QtGui/QPushButton>
#include <Qt3Support/Q3HBoxLayout>
#include <Qt3Support/Q3VBoxLayout>
#include <QtCore/QDebug>

TextDataDialog::TextDataDialog(QWidget* parent)
    : QDialog(parent)
{
  setCaption(tr("TextData"));

  stnr = 0;

  textLabel0 = new QLabel(this);
  textLabel0->setText(tr("Station"));

  textLabel1 = new QLabel(this);
  textLabel1->setText(tr("Time range"));

  textLabel2 = new QLabel(this);
  textLabel2->setText(tr("From"));

  textLabel3 = new QLabel(this);
  textLabel3->setText(tr("To"));

  stationEdit = new QLineEdit(this);

  QDateTime ldtto = timeutil::nowWithMinutes0Seconds0();
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

  okButton = new QPushButton(tr("OK"), this);
  okButton->setGeometry(20, 620, 90, 30);
  okButton->setFont(QFont("Arial", 9));

  cancelButton = new QPushButton(tr("Avbryt"), this);
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

  connect(cancelButton, SIGNAL(clicked()), this, SLOT(hide()));
  connect(okButton,     SIGNAL(clicked()), this, SLOT(checkStationId()));
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

TimeSpan TextDataDialog::getTimeSpan()
{
    TimeSpan ret;
    ret.first = fromEdit->dateTime();
    ret.second = timeutil::nowWithMinutes0Seconds0();
    return ret;
}

void TextDataDialog::checkStationId()
{
    if (KvMetaDataBuffer::instance()->isKnownStation(stnr)) {
        hide();
        /*emit*/ textDataApply();
    } else {
        QMessageBox::information( this, tr("TextData"),
                                  tr("Illegal station number. Choose a different station number."));
    }
}
