#include "textdatadialog.h"
#include "hqc_utilities.hh"
#include "MiDateTimeEdit.hh"

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/qmessagebox.h>
#include <QtGui/QPushButton>
#include <Qt3Support/Q3HBoxLayout>
#include <Qt3Support/Q3VBoxLayout>
#include <QtCore/QDebug>

TextDataDialog::TextDataDialog(const std::list<kvalobs::kvStation>& slist, QWidget* parent)
    : QDialog(parent)
    , stationList(slist)
{
  setCaption(tr("TextData"));

  stnr = 0;

  textLabel0 = new QLabel(this);
  textLabel0->setText(tr("Stasjon"));

  textLabel1 = new QLabel(this);
  textLabel1->setText(tr("Tidsrom"));

  textLabel2 = new QLabel(this);
  textLabel2->setText(tr("Fra"));

  textLabel3 = new QLabel(this);
  textLabel3->setText(tr("Til"));

  stationEdit = new QLineEdit(this);

  QDateTime ldtto(QDate::currentDate(), QTime::currentTime(), Qt::UTC);
  dtto = ldtto;
  dtfrom = dtto.addDays(-2);
  fromEdit = new MiDateTimeEdit(dtfrom,this);
  toEdit   = new MiDateTimeEdit(dtto,this);
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

void TextDataDialog::checkStationId()
{
    qDebug() << "n.stations=" << stationList.size();
    std::list<kvalobs::kvStation>::const_iterator it = std::find_if(stationList.begin(), stationList.end(), kvStationById(stnr));
    const bool legalStation = ( it != stationList.end() );
    if ( legalStation ) {
        emit textDataApply();
    } else {
        QMessageBox::information( this, tr("TextData"),
                tr("Ugyldig stasjonsnummer.\nVelg et annet stasjonsnummer."));
    }
}
