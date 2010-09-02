#include "rejectdialog.h"
#include <Q3HBoxLayout>
#include <QLabel>
//#include <Q3GridLayout>
#include <Q3VBoxLayout>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QPushButton>

RejectDialog::RejectDialog(QWidget* parent): QDialog(parent) {  
  
  setCaption("Forkastet");
  textLabel1 = new QLabel(this);
  textLabel1->setText("Velg tidsrom for forkastetliste");
  
  textLabel2 = new QLabel(this);
  textLabel2->setText("Fra");
  
  textLabel3 = new QLabel(this);
  textLabel3->setText("Til");
  
  
  //  miutil::miTime d = 
  QDateTime ldtto(QDate::currentDate(), QTime::currentTime(), Qt::UTC);
  dtto = ldtto;
  dtfrom = dtto.addDays(-2);
  fromEdit = new QDateTimeEdit(dtfrom,this);
  toEdit   = new QDateTimeEdit(dtto,this);
  fromEdit->setMaximumDate(dtto.date());
  fromEdit->setMaximumTime(dtto.time());
  fromEdit->setDisplayFormat("yyyy.MM.dd HH:mm");
  toEdit->setMinimumDate(dtfrom.date());
  toEdit->setMinimumTime(dtfrom.time());
  toEdit->setDisplayFormat("yyyy.MM.dd HH:mm");

  connect(fromEdit, SIGNAL(dateTimeChanged(const QDateTime&)),
	  this, SLOT(setFromTime(const QDateTime&)));

  connect(toEdit, SIGNAL(dateTimeChanged(const QDateTime&)),
	  this, SLOT(setToTime(const QDateTime&)));

  okButton = new QPushButton("OK", this);
  okButton->setGeometry(20, 620, 90, 30);
  okButton->setFont(QFont("Arial", 9));
  
  cancelButton = new QPushButton("Avbryt", this);
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

  connect(cancelButton,SIGNAL(clicked()), this, SIGNAL(rejectHide()));
  connect(okButton,SIGNAL(clicked()), this, SIGNAL(rejectApply()));
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
    ret.second = QDateTime::currentDateTime();
    return ret;
}
