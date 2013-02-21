
#include "rejectdialog.h"

#include "MiDateTimeEdit.hh"
#include "timeutil.hh"

#include <Qt3Support/Q3HBoxLayout>
#include <Qt3Support/Q3VBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>

RejectDialog::RejectDialog(QWidget* parent): QDialog(parent) {

  setCaption(tr("RejectDecode"));
  textLabel1 = new QLabel(this);
  textLabel1->setText(tr("Select time range for reject list"));

  textLabel2 = new QLabel(this);
  textLabel2->setText(tr("From"));

  textLabel3 = new QLabel(this);
  textLabel3->setText(tr("To"));

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

  connect(fromEdit, SIGNAL(dateTimeChanged(const QDateTime&)),
	  this, SLOT(setFromTime(const QDateTime&)));

  connect(toEdit, SIGNAL(dateTimeChanged(const QDateTime&)),
	  this, SLOT(setToTime(const QDateTime&)));

  okButton = new QPushButton(tr("OK"), this);
  okButton->setGeometry(20, 620, 90, 30);
  okButton->setFont(QFont("Arial", 9));

  cancelButton = new QPushButton(tr("Cancel"), this);
  cancelButton->setGeometry(120, 620, 90, 30);
  cancelButton->setFont(QFont("Arial", 9));

  QVBoxLayout* topLayout = new QVBoxLayout(this);
  QHBoxLayout* fromLayout = new QHBoxLayout();
  QHBoxLayout* toLayout = new QHBoxLayout();
  QHBoxLayout* buttonLayout = new QHBoxLayout();

  fromLayout->addWidget(textLabel2);
  fromLayout->addWidget(fromEdit);
  toLayout->addWidget(textLabel3);
  toLayout->addWidget(toEdit);
  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(cancelButton);

  topLayout->addWidget(textLabel1);
  topLayout->addLayout(fromLayout);
  topLayout->addLayout(toLayout);
  topLayout->addLayout(buttonLayout);

  connect(cancelButton, SIGNAL(clicked()), this, SLOT(hide()));
  connect(okButton,     SIGNAL(clicked()), this, SLOT(hide()));
  connect(okButton,     SIGNAL(clicked()), this, SIGNAL(rejectApply()));
}

void RejectDialog::setFromTime(const QDateTime& dt) {
  dtfrom = dt;
}

void RejectDialog::setToTime(const QDateTime& dt) {
  dtto = dt;
}

TimeSpan RejectDialog::getTimeSpan()
{
    TimeSpan ret;
    ret.first = fromEdit->dateTime();
    ret.second = timeutil::nowWithMinutes0Seconds0();
    return ret;
}
