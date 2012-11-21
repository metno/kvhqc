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
#include "dianashowdialog.h"

#include "HideApplyBox.hh"

#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>

DianaShowDialog::DianaShowDialog(QWidget* parent): QDialog(parent) {  

  setCaption(tr("Visning i Diana"));

  // Create a button group for parameter selection type
  
  paraTyp = new Q3ButtonGroup( 0, 
					  Qt::Horizontal, 
					  tr("HQC-synop"), this);

  Q3GridLayout* paraLayout = new Q3GridLayout(paraTyp->layout());

  // Insert checkbuttons for showtype selection
  QLabel* alLabel = new QLabel( tr("Fastsatt\nplass rundt\nstasjonsring"), paraTyp );
  QLabel* stLabel = new QLabel( tr("Standard\nparametere"), paraTyp );
  QLabel* atLabel = new QLabel( tr("Alternative\nparametere"), paraTyp );

  Q3ButtonGroup* taGroup = new Q3ButtonGroup();
  taType = new QRadioButton( "TA", paraTyp );
  taGroup->insert(taType);
  QLabel* taLabel = new QLabel( taType, "TTT   ", paraTyp );
  tamoType = new QRadioButton( "TA,modell", paraTyp );
  taGroup->insert(tamoType);
  tameType = new QRadioButton( "TAM", paraTyp );
  taGroup->insert(tameType);
  tadiType = new QRadioButton( tr("Differanse"), paraTyp );
  taGroup->insert(tadiType);

  tdGroup = new Q3ButtonGroup();
  tdType = new QRadioButton( "TD", paraTyp );
  tdGroup->insert(tdType);
  QLabel* tdLabel = new QLabel( tdType, "TdTdTd", paraTyp );
  uumoType = new QRadioButton( "UU,modell", paraTyp );
  tdGroup->insert(uumoType);
  uuType = new QRadioButton( "UU", paraTyp );
  tdGroup->insert(uuType);
  uumeType = new QRadioButton( "UUM", paraTyp );
  tdGroup->insert(uumeType);
  uumiType = new QRadioButton( "UUN", paraTyp );
  tdGroup->insert(uumiType);
  uumaType = new QRadioButton( "UUX", paraTyp );
  tdGroup->insert(uumaType);

  prGroup = new Q3ButtonGroup();
  prType = new QRadioButton( "PR", paraTyp );
  prGroup->insert(prType);
  QLabel* prLabel = new QLabel( prType, "PPPP  ", paraTyp );
  prmoType = new QRadioButton( "PR,modell", paraTyp );
  prGroup->insert(prmoType);
  poType = new QRadioButton( "PO", paraTyp );
  prGroup->insert(poType);
  pomeType = new QRadioButton( "POM", paraTyp );
  prGroup->insert(pomeType);
  pomiType = new QRadioButton( "PON", paraTyp );
  prGroup->insert(pomiType);
  pomaType = new QRadioButton( "POX", paraTyp );
  prGroup->insert(pomaType);
  phType = new QRadioButton( "PH", paraTyp );
  prGroup->insert(phType);
  podiType = new QRadioButton( tr("Differanse"), paraTyp );
  prGroup->insert(podiType);

  ppGroup = new Q3ButtonGroup();
  ppType = new QRadioButton( "PP", paraTyp );
  ppGroup->insert(ppType);
  QLabel* ppLabel = new QLabel( ppType, "PPP   ", paraTyp );
  ppmoType = new QRadioButton( "PP,modell", paraTyp );
  ppGroup->insert(ppmoType);

  rrGroup = new Q3ButtonGroup();
  rrType = new QRadioButton( "RR_12", paraTyp );
  rrGroup->insert(rrType);
  QLabel* rrLabel = new QLabel( rrType, "RRR  ", paraTyp );
  rrmoType = new QRadioButton( "RR_12,modell ", paraTyp );
  rrGroup->insert(rrmoType);
  rr1Type = new QRadioButton( "RR_1", paraTyp );
  rrGroup->insert(rr1Type);
  rr24Type = new QRadioButton( "RR_24", paraTyp );
  rrGroup->insert(rr24Type);
  rr24moType = new QRadioButton( "RR_24,modell", paraTyp );
  rrGroup->insert(rr24moType);
  rr6Type = new QRadioButton( "RR_6", paraTyp );
  rrGroup->insert(rr6Type);
  rrprType = new QRadioButton( tr("Forhold(%)"), paraTyp );
  rrGroup->insert(rrprType);

  tnxGroup = new Q3ButtonGroup();
  tn12Type = new QRadioButton( "TAN_12", paraTyp );
  tnxGroup->insert(tn12Type);
  QLabel* tnLabel = new QLabel( tn12Type, "TxTn     ", paraTyp );
  tx12Type = new QRadioButton( "TAX_12", paraTyp );
  tnxGroup->insert(tx12Type);
  tnType = new QRadioButton( "TAN", paraTyp );
  tnxGroup->insert(tnType);
  txType = new QRadioButton( "TAX", paraTyp );
  tnxGroup->insert(txType);

  ddGroup = new Q3ButtonGroup();
  ddType = new QRadioButton( "DD", paraTyp );
  ddGroup->insert(ddType);
  QLabel* ddLabel = new QLabel( ddType, "dd   ", paraTyp );
  ddmoType = new QRadioButton( "DD,modell", paraTyp );
  ddGroup->insert(ddmoType);

  ffGroup = new Q3ButtonGroup();
  ffType = new QRadioButton( "FF", paraTyp );
  ffGroup->insert(ffType);
  QLabel* ffLabel = new QLabel( ddType, "ff   ", paraTyp );
  ffmoType = new QRadioButton( "FF,modell", paraTyp );
  ffGroup->insert(ffmoType);

  fxGroup = new Q3ButtonGroup();
  fxType = new QRadioButton( "FX", paraTyp );
  fxGroup->insert(fxType);
  QLabel* fxLabel = new QLabel( ddType, "fxfx   ", paraTyp );
  fx01Type = new QRadioButton( "FX_1", paraTyp );
  fxGroup->insert(fx01Type);

  fgGroup = new Q3ButtonGroup();
  fgType = new QRadioButton( "FG", paraTyp );
  fgGroup->insert(fgType);
  QLabel* fgLabel = new QLabel( ddType, "FgFg ", paraTyp );
  fg01Type = new QRadioButton( "FG_1", paraTyp );
  fgGroup->insert(fg01Type);
  fg10Type = new QRadioButton( "FG_10", paraTyp );
  fgGroup->insert(fg10Type);

  saGroup = new Q3ButtonGroup();
  saType = new QRadioButton( "SA", paraTyp );
  saGroup->insert(saType);
  QLabel* saLabel = new QLabel( ddType, "sss  ", paraTyp );
  sdType = new QRadioButton( "SD", paraTyp );
  saGroup->insert(sdType);
  emType = new QRadioButton( "EM", paraTyp );
  saGroup->insert(emType);

  paraLayout->addWidget(alLabel ,0,0);
  paraLayout->addWidget(stLabel ,0,2);
  paraLayout->addWidget(atLabel ,0,4);

  paraLayout->addWidget(taLabel ,2,0);
  paraLayout->addWidget(taType  ,2,2);
  paraLayout->addWidget(tamoType,2,4);
  paraLayout->addWidget(tameType,2,5);
  paraLayout->addWidget(tadiType,2,6);

  paraLayout->addWidget(tdLabel ,4,0);
  paraLayout->addWidget(tdType  ,4,2);
  paraLayout->addWidget(uumoType,4,4);
  paraLayout->addWidget(uuType  ,4,5);
  paraLayout->addWidget(uumeType,4,6);
  paraLayout->addWidget(uumiType,5,5);
  paraLayout->addWidget(uumaType,5,6);

  paraLayout->addWidget(prLabel ,7,0);
  paraLayout->addWidget(prType  ,7,2);
  paraLayout->addWidget(prmoType,7,4);
  paraLayout->addWidget(poType  ,7,5);
  paraLayout->addWidget(pomeType,7,6);
  paraLayout->addWidget(pomiType,8,5);
  paraLayout->addWidget(pomaType,8,6);
  paraLayout->addWidget(phType  ,9,5);
  paraLayout->addWidget(podiType,9,6);

  paraLayout->addWidget(ppLabel ,11,0);
  paraLayout->addWidget(ppType  ,11,2);
  paraLayout->addWidget(ppmoType,11,4);

  paraLayout->addWidget(rrLabel   ,13,0);
  paraLayout->addWidget(rrType    ,13,2);
  paraLayout->addWidget(rrmoType  ,13,4);
  paraLayout->addWidget(rr1Type   ,13,5);
  paraLayout->addWidget(rr24Type  ,13,6);
  paraLayout->addWidget(rr24moType,14,4);
  paraLayout->addWidget(rr6Type   ,14,5);
  paraLayout->addWidget(rrprType  ,14,6);

  paraLayout->addWidget(tnLabel   ,16,0);
  paraLayout->addWidget(tn12Type  ,16,2);
  paraLayout->addWidget(tx12Type  ,18,2);
  paraLayout->addWidget(tnType    ,16,5);
  paraLayout->addWidget(txType    ,18,5);

  paraLayout->addWidget(ddLabel   ,20,0);
  paraLayout->addWidget(ddType    ,20,2);
  paraLayout->addWidget(ddmoType  ,20,4);

  paraLayout->addWidget(ffLabel   ,22,0);
  paraLayout->addWidget(ffType    ,22,2);
  paraLayout->addWidget(ffmoType  ,22,4);

  paraLayout->addWidget(fxLabel   ,24,0);
  paraLayout->addWidget(fxType    ,24,2);
  paraLayout->addWidget(fx01Type  ,24,4);

  paraLayout->addWidget(fgLabel   ,26,0);
  paraLayout->addWidget(fgType    ,26,2);
  paraLayout->addWidget(fg01Type  ,26,4);
  paraLayout->addWidget(fg10Type  ,26,5);

  paraLayout->addWidget(saLabel   ,28,0);
  paraLayout->addWidget(saType    ,28,2);
  paraLayout->addWidget(sdType    ,28,4);
  paraLayout->addWidget(emType    ,28,5);

  HideApplyBox* hab = new HideApplyBox(this);
  connect(hab, SIGNAL(hide()) , SIGNAL(dianaShowHide()));
  connect(hab, SIGNAL(apply()), SIGNAL(dianaShowApply()));

  Q3VBoxLayout* topLayout = new Q3VBoxLayout(this,10);
  topLayout->addWidget(paraTyp);
  topLayout->addWidget(hab);

  checkStandard();
}

//DianaShowDialog::~DianaShowDialog() {
//}

void DianaShowDialog::showAll(){
  this->show();
}

void DianaShowDialog::hideAll(){
  this->hide();
}

void DianaShowDialog::checkStandard() {
  taType->setChecked( TRUE );
  tdType->setChecked( TRUE );
  prType->setChecked( TRUE );
  ppType->setChecked( TRUE );
  rrType->setChecked( TRUE );
  tn12Type->setChecked( TRUE );
  ddType->setChecked( TRUE );
  ffType->setChecked( TRUE );
  fxType->setChecked( TRUE );
  fgType->setChecked( TRUE );
  saType->setChecked( TRUE );
}
