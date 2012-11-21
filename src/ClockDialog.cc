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

#include "HideApplyBox.hh"

#include <QBoxLayout>
#include <QGridLayout>

ClockDialog::ClockDialog(QWidget* parent): QDialog(parent) {  

  setCaption(tr("Tidspunkter"));

  // Create a button group
  
  QGroupBox* oTime = new QGroupBox(tr("Utvalgte tidspunkter"));
 
  QGridLayout* clockLayout = new QGridLayout;
  // insert checkbuttons for clocktime selection
  for ( int i = 0; i < 24; i++ ) {
    QString time;
    time.sprintf("%02d", i);
    clk[i] = new QCheckBox(time);
  }
  for ( int i = 0; i < 8; i++ ) {
    for( int j = 0; j < 3; j++ ) {
      clockLayout->addWidget(clk[3*i+j],i,j);
    }
  }
  oTime->setLayout(clockLayout);


  QGroupBox* staTime = new QGroupBox(tr("Tidspunkter"));
  QVBoxLayout* stdAllLayout = new QVBoxLayout;

  allTimes      = new QCheckBox(tr("&Alle tidspunkter"));
  standardTimes = new QCheckBox(tr("&Standardtidspunkter"));

  stdAllLayout->addWidget(allTimes);
  stdAllLayout->addWidget(standardTimes);
  staTime->setLayout(stdAllLayout);

  connect(allTimes, SIGNAL(clicked()),this,SLOT(standardCheck()));
  connect(standardTimes, SIGNAL(clicked()),this,SLOT(allCheck()));
  connect(allTimes, SIGNAL(clicked()),this,SLOT(oAllCheck()));
  connect(standardTimes, SIGNAL(clicked()),this,SLOT(oStandardCheck()));

  HideApplyBox* hab = new HideApplyBox(this);
  connect(hab, SIGNAL(hide()) , SIGNAL(ClockHide()));
  connect(hab, SIGNAL(apply()), SIGNAL(ClockApply()));

  QVBoxLayout* topLayout = new QVBoxLayout(this,10);
  topLayout->addWidget(staTime);
  topLayout->addWidget(oTime);
  topLayout->addWidget(hab);
}
void ClockDialog::showAll(){
  this->show();
}

void ClockDialog::hideAll(){
  this->hide();
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
