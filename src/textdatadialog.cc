#include "textdatadialog.h"
#include <Q3HBoxLayout>
#include <QLabel>
//#include <Q3GridLayout>
#include <Q3VBoxLayout>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QPushButton>
#include <qmessagebox.h>

TextDataDialog::TextDataDialog(vector<int> slist, QWidget* parent): QDialog(parent),stnrList(slist) {  
  
  setCaption("TextData");

  stnr = 0;

  textLabel0 = new QLabel(this);
  textLabel0->setText("Stasjon");

  textLabel1 = new QLabel(this);
  textLabel1->setText("Tidsrom");
  
  textLabel2 = new QLabel(this);
  textLabel2->setText("Fra");
  
  textLabel3 = new QLabel(this);
  textLabel3->setText("Til");
  
  stationEdit = new QLineEdit(this);  
  
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

  connect(stationEdit, SIGNAL(textChanged(const QString&)),
	  this, SLOT(setStation(const QString&)));

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

  connect(cancelButton,SIGNAL(clicked()), this, SIGNAL(textDataHide()));
  connect(okButton,SIGNAL(clicked()), this, SLOT(checkStationId()));
  //  connect(okButton,SIGNAL(clicked()), this, SIGNAL(textDataApply()));
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
    ret.second = QDateTime::currentDateTime();
    return ret;
}

void TextDataDialog::checkStationId() {
  bool legalStation = false;
  for ( int i = 0; i < stnrList.size(); i++) {
    if ( stnr == stnrList[i] ) {
      legalStation = true;
      break;
    }
  }
  if ( legalStation )
    emit textDataApply();
  else {
      QMessageBox::information( this, "TextData",
				"Ugyldig stasjonsnummer.\nVelg et annet stasjonsnummer.");
      return;
  }
}
