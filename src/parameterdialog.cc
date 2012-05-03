/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id$

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
#include "parameterdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

ParameterDialog::ParameterDialog(QWidget* parent): QDialog(parent) {  
  setCaption("Parametervalg");
  setGeometry(500,10,300,580);

  QVBoxLayout * vl = new QVBoxLayout(this,10);

  QGroupBox *pVal = new QGroupBox("Parametervalg", this);

  plb = new QListWidget(this);
  plb->setSelectionMode( QAbstractItemView::MultiSelection );
  connect(plb,SIGNAL(itemPressed(QListWidgetItem*)),SLOT(listClickedItem(QListWidgetItem*)));

  allPar    = new QRadioButton( "Velg alle parametere", pVal );
  allPar->setChecked(true);
  markPar   = new QRadioButton( "Velg merkede parametere", pVal );
  markPar->setDisabled(true);
  noMarkPar = new QRadioButton( "Velg bort merkede parametere", pVal );
  noMarkPar->setDisabled(true);

  QVBoxLayout * rbvl = new QVBoxLayout(this,10);
  rbvl->addWidget(allPar);
  rbvl->addWidget(markPar);
  rbvl->addWidget(noMarkPar);

  sthide = new QPushButton("Skjul", this);
  sthide->setGeometry(20, 620, 90, 30);
  sthide->setFont(QFont("Arial", 9));
  
  excu = new QPushButton("Utfør", this);
  excu->setGeometry(120, 620, 90, 30);
  excu->setFont(QFont("Arial", 9));
  
  hdnexcu = new QPushButton("Utfør+Skjul", this);
  hdnexcu->setGeometry(220, 620, 90, 30);
  hdnexcu->setFont(QFont("Arial", 9));
  hdnexcu->setDefault(true);

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(sthide, 10);
  buttonLayout->addWidget(excu, 10);
  buttonLayout->addWidget(hdnexcu, 10);

  connect(sthide, SIGNAL(clicked()), this, SIGNAL( paramHide()));
  connect(hdnexcu, SIGNAL(clicked()), this, SLOT( applyHideClicked()));
  connect(excu, SIGNAL(clicked()), this, SIGNAL( paramApply()));

  vl->addWidget(plb);

  vl->addWidget(pVal);

  vl->addLayout(rbvl);

  vl->addLayout(buttonLayout);

  hdnexcu->setFocus();
}

void ParameterDialog::insertParametersInListBox(const std::vector<int> & porder, const QMap<int,QString> & parMap) {
  plb->clear();
  int jj = 0;
  for (std::vector<int>::const_iterator it = porder.begin(); it != porder.end(); ++it) {
      QString sp = parMap[*it];
      //      plb->insertItem(sp);
      plb->insertItem(jj,sp);
      jj++;
  }
}


void ParameterDialog::showAll(){
  allPar->setChecked(true);
  markPar->setDisabled(true);
  noMarkPar->setDisabled(true);
  this->show();
}

void ParameterDialog::hideAll(){
  this->hide();
}

void ParameterDialog::applyHideClicked(){
  emit paramHide();
  emit paramApply();
}

//void ParameterDialog::listClickedItem(Q3ListBoxItem* lbItem) {
void ParameterDialog::listClickedItem(QListWidgetItem* lbItem) {
  markPar->setDisabled(false);
  noMarkPar->setDisabled(false);
}
