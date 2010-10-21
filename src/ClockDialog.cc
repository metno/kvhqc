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
#include "ClockDialog.h"
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>

ClockDialog::ClockDialog(QWidget* parent): QDialog(parent) {  

  setCaption("Tidspunkter");

  // Create a button group
  
  Q3ButtonGroup* oTime = new Q3ButtonGroup( 0, 
					  Qt::Horizontal, 
					  "Tidspunkter", this);
  Q3GridLayout* clockLayout = new Q3GridLayout(oTime->layout());
  // insert checkbuttons for clocktime selection
  for ( int i = 0; i < 24; i++ ) {
    QString time;
    time.sprintf("%02d", i);
    clk[i] = new QCheckBox(time, oTime);
  }
  for ( int i = 0; i < 8; i++ ) {
    for( int j = 0; j < 3; j++ ) {
      clockLayout->addWidget(clk[3*i+j],i,j);
    }
  }

  Q3ButtonGroup* staTime = new Q3ButtonGroup( 1, 
					  Qt::Horizontal, 
					  "Utvalgte tidspunkter", this);

  allTimes      = new QCheckBox("&Alle tidspunkter", staTime);
  standardTimes = new QCheckBox("&Standardtidspunkter", staTime);

  connect(allTimes, SIGNAL(clicked()),this,SLOT(standardCheck()));
  connect(standardTimes, SIGNAL(clicked()),this,SLOT(allCheck()));
  connect(allTimes, SIGNAL(clicked()),this,SLOT(oAllCheck()));
  connect(standardTimes, SIGNAL(clicked()),this,SLOT(oStandardCheck()));

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

  Q3HBoxLayout* buttonLayout = new Q3HBoxLayout();
  buttonLayout->addWidget(sthide, 10);
  buttonLayout->addWidget(excu, 10);
  buttonLayout->addWidget(hdnexcu, 10);

  connect(sthide, SIGNAL(clicked()), this, SIGNAL( ClockHide()));
  connect(hdnexcu, SIGNAL(clicked()), this, SLOT( applyHideClicked()));
  connect(excu, SIGNAL(clicked()), this, SIGNAL( ClockApply()));

  Q3VBoxLayout* topLayout = new Q3VBoxLayout(this,10);
  topLayout->addWidget(staTime);
  topLayout->addWidget(oTime);
  topLayout->addLayout(buttonLayout);
}
void ClockDialog::showAll(){
  this->show();
}

void ClockDialog::hideAll(){
  this->hide();
}

void ClockDialog::applyHideClicked(){
  emit ClockHide();
  emit ClockApply();
}

void ClockDialog::standardCheck() {
  if ( allTimes->isChecked() )
    standardTimes->setChecked( FALSE );
}

void ClockDialog::allCheck() {
  if ( standardTimes->isChecked() )
    allTimes->setChecked( FALSE );
}

void ClockDialog::oStandardCheck() {
  for ( int i = 0; i < 24; i++ )
    clk[i]->setChecked( FALSE );
  if ( standardTimes->isChecked() ) {
    for ( int i = 0; i < 24; i += 3 )
      clk[i]->setChecked( TRUE );
  }
}

void ClockDialog::oAllCheck() {
  for ( int i = 0; i < 24; i++ )
    clk[i]->setChecked( FALSE );
  if ( allTimes->isChecked() ) {
    for ( int i = 0; i < 24; i++ )
      clk[i]->setChecked( TRUE );
  }
}
