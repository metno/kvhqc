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
#include "accepttimeseriesdialog.h"

#include "HideApplyBox.hh"
#include "timeutil.hh"

#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

AcceptTimeseriesDialog::AcceptTimeseriesDialog(): QDialog() 
{  
  setCaption(tr("Godkjenn tidsserie"));

  QLabel* statLabel = new QLabel(tr("Stasjon"), this );
  stationWidget   = new QListWidget(this);
  connect(stationWidget, SIGNAL(itemClicked(QListWidgetItem*)),
	  this, SLOT(stationSelected(QListWidgetItem*)));

  QLabel* paraLabel = new QLabel(tr("Parameter"), this );
  parameterWidget = new QListWidget(this);
  connect(parameterWidget, SIGNAL(itemClicked(QListWidgetItem*)),
	  this, SLOT(parameterSelectionChanged(QListWidgetItem*)));

  QLabel* resultLabel = new QLabel( "Valgt tidsserie", this );
  resultWidget = new QListWidget(this);
  resultWidget->setFixedHeight(30);

  qc2Val = new QGroupBox(this);
  qc2Yes    = new QRadioButton(tr("Retting av QC2 mulig"), qc2Val );
  qc2Yes->setChecked(true);
  qc2No   = new QRadioButton(tr("Retting av QC2 IKKE mulig"), qc2Val );

  QDateTime t = timeutil::nowWithMinutes0Seconds0();
  QDateTime f = t.addSecs(-2*24*3600 + 3600*(17-t.time().hour()) + 60*45);

  QLabel* fromLabel = new QLabel(tr("Fra"), this );
  fromTimeEdit  = new MiDateTimeEdit(f,this);
  fromTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm");

  QLabel* toLabel   = new QLabel(tr("Til"), this );
  toTimeEdit    = new MiDateTimeEdit(t,this);
  toTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm");

  HideApplyBox* hab = new HideApplyBox(this);

  QHBoxLayout* stationLayout = new QHBoxLayout();
  stationLayout->addWidget(statLabel, 10);
  stationLayout->addWidget(stationWidget, 10);
  QHBoxLayout* parameterLayout = new QHBoxLayout();
  parameterLayout->addWidget(paraLabel, 10);
  parameterLayout->addWidget(parameterWidget, 10);
  QVBoxLayout * vl = new QVBoxLayout();
  vl->addWidget(qc2Yes);
  vl->addWidget(qc2No);
  QHBoxLayout* fromLayout = new QHBoxLayout();
  fromLayout->addWidget(fromLabel, 10);
  fromLayout->addWidget(fromTimeEdit, 10);
  QHBoxLayout* toLayout = new QHBoxLayout();
  toLayout->addWidget(toLabel, 10);
  toLayout->addWidget(toTimeEdit, 10);

  QVBoxLayout* topLayout = new QVBoxLayout(this,10);
  topLayout->addWidget(statLabel);
  topLayout->addWidget(stationWidget);
  topLayout->addWidget(paraLabel);
  topLayout->addWidget(parameterWidget);
  topLayout->addWidget(resultLabel);
  topLayout->addWidget(resultWidget);
  topLayout->addLayout(vl);
  topLayout->addLayout(fromLayout);
  topLayout->addLayout(toLayout);
  topLayout->addWidget(hab);

  connect(hab, SIGNAL(hide()),  SIGNAL(tsAcceptHide()));
  connect(hab, SIGNAL(apply()), SIGNAL(tsAcceptApply()));
}

void AcceptTimeseriesDialog::showAll(){
  this->show();
}

void AcceptTimeseriesDialog::hideAll(){
  this->hide();
}

void AcceptTimeseriesDialog::newParameterList(const QStringList& parameterList)
{
  int n = parameterList.size();
  for(int i=0; i<n; i++ ){
    new QListWidgetItem(parameterList[i], parameterWidget);
  }
}

void AcceptTimeseriesDialog::newStationList(std::vector<QString>& stationList)
{
  int n = stationList.size();
  for(int i=0; i<n; i++ ){
    new QListWidgetItem(stationList[i], stationWidget);
  }
}

void AcceptTimeseriesDialog::stationSelected(QListWidgetItem * item) {
  if( parameterWidget->currentItem() == 0 ) return;
  QString str = item->text();
  str = str.trimmed();
  str+= " ";
  str += parameterWidget->currentItem()->text();
  resultWidget->clear();
  resultWidget->insertItem(0, str);
}

void AcceptTimeseriesDialog::parameterSelectionChanged(QListWidgetItem *item) {
  if(parameterWidget->currentItem() == 0 ) return;
  resultWidget->clear();
  QString str = stationWidget->currentItem()->text();
  str = str.trimmed();
  str+= " ";
  str += item->text();
  resultWidget->insertItem(0, str);
}

bool AcceptTimeseriesDialog::getResults(QString& parameter,
					QDateTime& fromTime,
					QDateTime& toTime,
					int& stationID,
					bool& maybeQC2)
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
  maybeQC2 = qc2Yes->isChecked(); 
  return true;
}
