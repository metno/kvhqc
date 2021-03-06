
#include "RejectedObsDialog.hh"

#include "util/timeutil.hh"
#include "util/gui/MiDateTimeEdit.hh"

#include <QtCore/QEvent>
#include <Qt3Support/Q3HBoxLayout>
#include <Qt3Support/Q3VBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>

RejectedObsDialog::RejectedObsDialog(QWidget* parent)
  : QDialog(parent)
{
  textLabel1 = new QLabel(this);
  textLabel2 = new QLabel(this);
  textLabel3 = new QLabel(this);

  QDateTime dtto = timeutil::nowWithMinutes0Seconds0();
  QDateTime dtfrom = dtto.addDays(-2);
  fromEdit = new MiDateTimeEdit(dtfrom,this);
  toEdit   = new MiDateTimeEdit(dtto,this);
  fromEdit->setMaximumDate(dtto.date());
  fromEdit->setMaximumTime(dtto.time());
  fromEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
  toEdit->setMinimumDate(dtfrom.date());
  toEdit->setMinimumTime(dtfrom.time());
  toEdit->setDisplayFormat("yyyy-MM-dd hh:mm");

  okButton = new QPushButton(this);
  okButton->setGeometry(20, 620, 90, 30);
  okButton->setFont(QFont("Arial", 9));

  cancelButton = new QPushButton(this);
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

  retranslateUi();

  connect(cancelButton, SIGNAL(clicked()), this, SLOT(hide()));
  connect(okButton,     SIGNAL(clicked()), this, SLOT(hide()));
  connect(okButton,     SIGNAL(clicked()), this, SIGNAL(rejectApply()));
}

void RejectedObsDialog::retranslateUi()
{
  setCaption(tr("RejectDecode"));
  textLabel1->setText(tr("Select time range for reject list"));
  textLabel2->setText(tr("From"));
  textLabel3->setText(tr("To"));
  okButton->setText(tr("OK"));
  cancelButton->setText(tr("Cancel"));
}

void RejectedObsDialog::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QDialog::changeEvent(event);
}

TimeRange RejectedObsDialog::getTimeRange() const
{
  const timeutil::ptime f = timeutil::from_QDateTime(fromEdit->dateTime());
  const timeutil::ptime t = timeutil::from_QDateTime(toEdit  ->dateTime());
  return TimeRange(f, t);
}
