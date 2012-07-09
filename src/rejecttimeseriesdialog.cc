/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id: dianashowdialog.cc 684 2011-04-12 11:55:00Z knutj $

Copyright (C) 2007 met.no

Contact information:
Norwegian Meteorological Institute
Box 43 Blindern
0313 OSLO
NORWAY
email: kvalobs-dev@met.no

This file is part of HQC

HQC is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

HQC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with HQC; if not, write to the Free Software Foundation Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "rejecttimeseriesdialog.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <iostream>

using namespace std;

RejectTimeseriesDialog::RejectTimeseriesDialog(): QDialog() 
{  

  setCaption(tr("Forkast tidsserie"));

  QLabel* statLabel = new QLabel( tr("Stasjon"), this );
  stationWidget   = new QListWidget(this);
  connect(stationWidget, SIGNAL(itemClicked(QListWidgetItem*)),
	  this, SLOT(stationSelected(QListWidgetItem*)));

  QLabel* paraLabel = new QLabel( tr("Parameter"), this );
  parameterWidget = new QListWidget(this);
  connect(parameterWidget, SIGNAL(itemClicked(QListWidgetItem*)),
	  this, SLOT(parameterSelectionChanged(QListWidgetItem*)));

  QLabel* resultLabel = new QLabel( tr("Valgt tidsserie"), this );
  resultWidget = new QListWidget(this);
  resultWidget->setFixedHeight(30);

  QLabel* fromLabel = new QLabel( tr("Fra"), this );
  fromTimeEdit  = new QDateTimeEdit(QDateTime::currentDateTime(),this);
  fromTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm");

  QLabel* toLabel   = new QLabel( tr("Til"), this );
  toTimeEdit    = new QDateTimeEdit(QDateTime::currentDateTime(),this);
  toTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm");

  QDateTime t(toTimeEdit->dateTime());

  if( t.time().minute() != 0 ){
    t = t.addSecs(-60*t.time().minute());
  }
  toTimeEdit->setDateTime(t);

  t = t.addSecs(-172800); // Go back two days 
  t = t.addSecs(3600*(17-t.time().hour()));
  t = t.addSecs(60*(45-t.time().minute()));
  fromTimeEdit->setDateTime(t);

  sthide = new QPushButton(tr("Skjul"), this);
  sthide->setGeometry(20, 620, 90, 30);
  sthide->setFont(QFont("Arial", 9));

  excu = new QPushButton(tr("Utf�r"), this);
  excu->setGeometry(120, 620, 90, 30);
  excu->setFont(QFont("Arial", 9));
  
  hdnexcu = new QPushButton(tr("Utf�r+Skjul"), this);
  hdnexcu->setGeometry(220, 620, 90, 30);
  hdnexcu->setFont(QFont("Arial", 9));
  hdnexcu->setDefault(true);

  QHBoxLayout* stationLayout = new QHBoxLayout();
  stationLayout->addWidget(statLabel, 10);
  stationLayout->addWidget(stationWidget, 10);
  QHBoxLayout* parameterLayout = new QHBoxLayout();
  parameterLayout->addWidget(paraLabel, 10);
  parameterLayout->addWidget(parameterWidget, 10);
  QHBoxLayout* fromLayout = new QHBoxLayout();
  fromLayout->addWidget(fromLabel, 10);
  fromLayout->addWidget(fromTimeEdit, 10);
  QHBoxLayout* toLayout = new QHBoxLayout();
  toLayout->addWidget(toLabel, 10);
  toLayout->addWidget(toTimeEdit, 10);
  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(sthide, 10);
  buttonLayout->addWidget(excu, 10);
  buttonLayout->addWidget(hdnexcu, 10);

  QVBoxLayout* topLayout = new QVBoxLayout(this,10);
  topLayout->addWidget(statLabel);
  topLayout->addWidget(stationWidget);
  topLayout->addWidget(paraLabel);
  topLayout->addWidget(parameterWidget);
  topLayout->addWidget(resultLabel);
  topLayout->addWidget(resultWidget);
  topLayout->addLayout(fromLayout);
  topLayout->addLayout(toLayout);
  topLayout->addLayout(buttonLayout);

  connect(sthide,      SIGNAL(clicked()), SIGNAL( tsRejectHide()));
  connect(hdnexcu,     SIGNAL(clicked()), SIGNAL( tsRejectHide()));
  connect(hdnexcu,     SIGNAL(clicked()), SIGNAL( tsRejectApply()));
  connect(excu,        SIGNAL(clicked()), SIGNAL( tsRejectApply()));
}

void RejectTimeseriesDialog::showAll(){
  this->show();
}

void RejectTimeseriesDialog::hideAll(){
  this->hide();
}

void RejectTimeseriesDialog::newParameterList(const QStringList& parameterList)
{
  int n = parameterList.size();
  for(int i=0; i<n; i++ ){
    new QListWidgetItem(parameterList[i], parameterWidget);
  }
}

void RejectTimeseriesDialog::newStationList(std::vector<QString>& stationList)
{
  int n = stationList.size();
  for(int i=0; i<n; i++ ){
    new QListWidgetItem(stationList[i], stationWidget);
  }
}

void RejectTimeseriesDialog::stationSelected(QListWidgetItem * item) {
  if( parameterWidget->currentItem() == 0 ) return;
  QString str = item->text();
  str = str.trimmed();
  str+= " ";
  str += parameterWidget->currentItem()->text();
  resultWidget->clear();
  resultWidget->insertItem(0, str);
}

void RejectTimeseriesDialog::parameterSelectionChanged(QListWidgetItem *item) {
  if(parameterWidget->currentItem() == 0 ) return;
  resultWidget->clear();
  QString str = stationWidget->currentItem()->text();
  str = str.trimmed();
  str+= " ";
  str += item->text();
  resultWidget->insertItem(0, str);
}

bool RejectTimeseriesDialog::getResults(QString& parameter,
					QDateTime& fromTime,
					QDateTime& toTime,
					int& stationID)
{
  fromTime = fromTimeEdit->dateTime();
  toTime   = toTimeEdit->dateTime();
  if ( resultWidget->item(0) == 0 ) return false;
  QString result = resultWidget->item(0)->text();
  int bl1 = result.indexOf(" ");
  int bl2 = result.lastIndexOf(" ") + 1;
  int l = result.length();
  QString strStation = result.left(bl1);
  bool ok;
  stationID = strStation.toInt(&ok, 10);
  parameter = result.right(l -bl2);
  return true;
}
