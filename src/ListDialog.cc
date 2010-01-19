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
#include "ListDialog.h"
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3GridLayout>
#include <Q3VBoxLayout>

ListDialog::ListDialog(QWidget* parent): QDialog(parent) {  

  setCaption("Datautvalg HQC");

  // Create a button group for control type
  
  Q3ButtonGroup *ctrlTyp = new Q3ButtonGroup( 1, 
					  Qt::Horizontal, 
					  "Kontrolltype", this);
  Q3GridLayout* controlLayout = new Q3GridLayout(ctrlTyp->layout());

  // insert checkbuttons for control type selection
  twiType = new QCheckBox( "Temperatur,fuktighet", ctrlTyp );
  prcType = new QCheckBox( "Nedbør,snøforhold", ctrlTyp );
  aprType = new QCheckBox( "Lufttrykk", ctrlTyp );
  winType = new QCheckBox( "Vind", ctrlTyp );
  marType = new QCheckBox( "Maritime parametere", ctrlTyp );
  visType = new QCheckBox( "Visuelle parametere", ctrlTyp );

  connect(twiType,SIGNAL(clicked()),this,SLOT(twiCheck()));
  connect(twiType,SIGNAL(clicked()),this,SLOT(otwiCheck()));
  connect(prcType,SIGNAL(clicked()),this,SLOT(prcCheck()));
  connect(prcType,SIGNAL(clicked()),this,SLOT(oprcCheck()));
  connect(aprType,SIGNAL(clicked()),this,SLOT(aprCheck()));
  connect(aprType,SIGNAL(clicked()),this,SLOT(oaprCheck()));
  connect(winType,SIGNAL(clicked()),this,SLOT(winCheck()));
  connect(winType,SIGNAL(clicked()),this,SLOT(owinCheck()));
  connect(visType,SIGNAL(clicked()),this,SLOT(visCheck()));
  connect(visType,SIGNAL(clicked()),this,SLOT(ovisCheck()));
  connect(marType,SIGNAL(clicked()),this,SLOT(marCheck()));
  connect(marType,SIGNAL(clicked()),this,SLOT(omarCheck()));

  // Create a button group for station type
  
  Q3ButtonGroup *stTyp = new Q3ButtonGroup(0, Qt::Horizontal , 
					 "Stasjonstype", 
					 this);
  Q3GridLayout* statSelLayout = new Q3GridLayout(stTyp->layout());

  // insert checkbuttons for station type selection
  aaType = new QCheckBox( "AA", stTyp );
  afType = new QCheckBox( "AF", stTyp );
  alType = new QCheckBox( "AL", stTyp );
  avType = new QCheckBox( "AV", stTyp );
  aoType = new QCheckBox( "AO", stTyp );
  aeType = new QCheckBox( "AE", stTyp );
  mvType = new QCheckBox( "MV", stTyp );
  mpType = new QCheckBox( "MP", stTyp );
  mmType = new QCheckBox( "MM", stTyp );
  msType = new QCheckBox( "MS", stTyp );
  fmType = new QCheckBox( "FM", stTyp );
  nsType = new QCheckBox( "NS", stTyp );
  ndType = new QCheckBox( "ND", stTyp );
  noType = new QCheckBox( "NO", stTyp );
  piType = new QCheckBox( "P ", stTyp );
  ptType = new QCheckBox( "PT", stTyp );
  vsType = new QCheckBox( "VS", stTyp );
  vkType = new QCheckBox( "VK", stTyp );
  vmType = new QCheckBox( "VM", stTyp );
  allType = new QCheckBox( "Alle", stTyp );


   // Create a button group for station location (county)
  
  Q3ButtonGroup *stCounty = new Q3ButtonGroup( 0, 
					  Qt::Horizontal, 
					  "Fylke", this);
  Q3GridLayout* statCountyLayout = new Q3GridLayout(stCounty->layout());

  // insert checkbuttons for station location selection
  oslCoun = new QCheckBox( "Oslo", stCounty );
  akeCoun = new QCheckBox( "Akershus", stCounty );
  ostCoun = new QCheckBox( "Østfold", stCounty );
  hedCoun = new QCheckBox( "Hedmark", stCounty );
  oppCoun = new QCheckBox( "Oppland", stCounty );
  busCoun = new QCheckBox( "Buskerud", stCounty );
  vefCoun = new QCheckBox( "Vestfold", stCounty );
  telCoun = new QCheckBox( "Telemark", stCounty );
  ausCoun = new QCheckBox( "Aust-Agder", stCounty );
  veaCoun = new QCheckBox( "Vest-Agder", stCounty );
  rogCoun = new QCheckBox( "Rogaland", stCounty );
  horCoun = new QCheckBox( "Hordaland", stCounty );
  sogCoun = new QCheckBox( "Sogn og Fjordane", stCounty );
  morCoun = new QCheckBox( "Møre og Romsdal", stCounty );
  sorCoun = new QCheckBox( "Sør-Trøndelag", stCounty );
  ntrCoun = new QCheckBox( "Nord-Trøndelag", stCounty );
  norCoun = new QCheckBox( "Nordland", stCounty );
  troCoun = new QCheckBox( "Troms", stCounty );
  finCoun = new QCheckBox( "Finnmark", stCounty );
  svaCoun = new QCheckBox( "Ishavet", stCounty );
  allCoun = new QCheckBox( "Alle", stCounty );

   // Create a button group for station location (region)
  
  Q3ButtonGroup *stRegion = new Q3ButtonGroup( 1, 
					  Qt::Horizontal, 
					  "Landsdel", this);

  // insert checkbuttons for station location selection
  ausReg = new QCheckBox( "Østlandet ", stRegion );
  vesReg = new QCheckBox( "Vestlandet", stRegion );
  troReg = new QCheckBox( "Trøndelag ", stRegion );
  norReg = new QCheckBox( "Nord-Norge", stRegion );
  webReg = new QCheckBox( "Synop-stasjoner", stRegion );
  priReg = new QCheckBox( "Prioriterte stasjoner", stRegion );

  connect(ausReg,SIGNAL(clicked()), this,SLOT(ausCheck()));
  connect(ausReg,SIGNAL(clicked()), this,SLOT(oausCheck()));
  connect(vesReg,SIGNAL(clicked()), this,SLOT(vesCheck()));
  connect(vesReg,SIGNAL(clicked()), this,SLOT(ovesCheck()));
  connect(troReg,SIGNAL(clicked()), this,SLOT(troCheck()));
  connect(troReg,SIGNAL(clicked()), this,SLOT(otroCheck()));
  connect(norReg,SIGNAL(clicked()), this,SLOT(norCheck()));
  connect(norReg,SIGNAL(clicked()), this,SLOT(onorCheck()));
  connect(webReg,SIGNAL(clicked()), this,SLOT(webCheck()));
  connect(webReg,SIGNAL(clicked()), this,SLOT(owebCheck()));
  connect(priReg,SIGNAL(clicked()), this,SLOT(priCheck()));
  connect(priReg,SIGNAL(clicked()), this,SLOT(opriCheck()));
  connect(allCoun,SIGNAL(clicked()), this,SLOT(allCounCheck()));
  connect(oslCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(akeCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(ostCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(hedCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(oppCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(busCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(vefCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(telCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(ausCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(veaCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(rogCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(horCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(sogCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(morCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(sorCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(ntrCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(norCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(troCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));
  connect(finCoun,SIGNAL(clicked()), this,SLOT(allCounUnCheck()));

  // Typeid options
  Q3ButtonGroup *typeGroup = new Q3ButtonGroup( 1, 
					  Qt::Horizontal, 
					  "Meldingstyper", this);
  
  Q3GridLayout* typeidLayout = new Q3GridLayout(typeGroup->layout());
  priTypes = new QRadioButton( "Prioriterte typer", typeGroup );
  typeGroup->insert(priTypes);
  priTypes->setChecked(true);
  allTypes = new QRadioButton( "Alle typer", typeGroup );
  typeGroup->insert(allTypes);
  
  //Station selection
  stationSelect = new QPushButton("Velg stasjon", this);
  stationSelect->setGeometry(10, 110, 400, 30);
  stationSelect->setFont(QFont("Arial", 9));
  connect(stationSelect, SIGNAL(clicked()), this, SIGNAL( selectStation()));

  stationLabel = new QLabel(this);
  stationLabel->setText("Valgte stasjoner");
  stationLabel->setFont(QFont("Arial", 12));
  stationLabel->setPaletteForegroundColor(Qt::darkBlue);
  stationLabel->setAlignment(Qt::AlignLeft);
  stationNames = new Q3ListBox(this);

  //Time selection  
  fromTime = new miTimeSpinBox ("fromTime",this, "Fra: ");
  toTime   = new miTimeSpinBox ("toTime",this, "Til:  ");
  //  fromTime = new QDateTimeEdit (QDateTime::currentDateTime(),this);
  //  toTime   = new QDateTimeEdit (QDateTime::currentDateTime(),this);
  miutil::miTime t(toTime->time());
  if( t.min() != 0 ){
    t.addMin(-1*t.min());
    t.addHour(1);
    toTime->setTime(t);
  }
  t.addDay(-2);
  t.addHour(17-t.hour());
  t.addMin(45-t.min());
  fromTime->setTime(t);

  connect( fromTime, SIGNAL(valueChanged(const miutil::miTime&)),
	   toTime,   SLOT(  setMin(const miutil::miTime&)     ));
  connect( fromTime, SIGNAL(valueChanged(const miutil::miTime&)),
	     this, SIGNAL(fromTimeChanged(const miutil::miTime&)));

  connect( toTime,  SIGNAL(valueChanged(const miutil::miTime&)),
	   fromTime,SLOT(  setMax(const miutil::miTime&)     ));
  connect( toTime,  SIGNAL(valueChanged(const miutil::miTime&)),
	     this,SIGNAL(toTimeChanged(const miutil::miTime&)));

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
  statSelLayout->addWidget(aaType,0,0);
  statSelLayout->addWidget(afType,1,0);
  statSelLayout->addWidget(alType,2,0);
  statSelLayout->addWidget(avType,3,0);
  statSelLayout->addWidget(aoType,4,0);
  statSelLayout->addWidget(aeType,5,0);
  statSelLayout->addWidget(mvType,0,1);
  statSelLayout->addWidget(mpType,1,1);
  statSelLayout->addWidget(mmType,2,1);
  statSelLayout->addWidget(msType,3,1);
  statSelLayout->addWidget(fmType,4,3);
  statSelLayout->addWidget(nsType,0,2);
  statSelLayout->addWidget(ndType,1,2);
  statSelLayout->addWidget(noType,2,2);
  statSelLayout->addWidget(piType,4,2);
  statSelLayout->addWidget(ptType,5,2);
  statSelLayout->addWidget(vsType,0,3);
  statSelLayout->addWidget(vkType,1,3);
  statSelLayout->addWidget(vmType,2,3);
  statSelLayout->addWidget(allType,5,3);

  statCountyLayout->addWidget(oslCoun,0,0);
  statCountyLayout->addWidget(hedCoun,1,0);
  statCountyLayout->addWidget(vefCoun,2,0);
  statCountyLayout->addWidget(veaCoun,3,0);
  statCountyLayout->addWidget(sogCoun,4,0);
  statCountyLayout->addWidget(ntrCoun,5,0);
  statCountyLayout->addWidget(finCoun,6,0);
  statCountyLayout->addWidget(akeCoun,0,1);
  statCountyLayout->addWidget(oppCoun,1,1);
  statCountyLayout->addWidget(telCoun,2,1);
  statCountyLayout->addWidget(rogCoun,3,1);
  statCountyLayout->addWidget(morCoun,4,1);
  statCountyLayout->addWidget(norCoun,5,1);
  statCountyLayout->addWidget(svaCoun,6,1);
  statCountyLayout->addWidget(ostCoun,0,2);
  statCountyLayout->addWidget(busCoun,1,2);
  statCountyLayout->addWidget(ausCoun,2,2);
  statCountyLayout->addWidget(horCoun,3,2);
  statCountyLayout->addWidget(sorCoun,4,2);
  statCountyLayout->addWidget(troCoun,5,2);
  statCountyLayout->addWidget(allCoun,6,2);

  Q3HBoxLayout* locationLayout = new Q3HBoxLayout();
  locationLayout->addWidget(stRegion, 10);
  locationLayout->addWidget(stCounty, 10);

  Q3HBoxLayout* typeLayout = new Q3HBoxLayout();
  typeLayout->addWidget(ctrlTyp, 10);
  typeLayout->addWidget(stTyp, 10);

  connect(sthide, SIGNAL(clicked()), this, SIGNAL( ListHide()));
  connect(hdnexcu, SIGNAL(clicked()), this, SLOT( applyHideClicked()));
  connect(excu, SIGNAL(clicked()), this, SIGNAL( ListApply()));

  topLayout = new Q3VBoxLayout(this,10);
  
  topLayout->addLayout(typeLayout);
  topLayout->addLayout(locationLayout);
  topLayout->addWidget(typeGroup);
  topLayout->addWidget(stationSelect);
  topLayout->addWidget(stationLabel);
  topLayout->addWidget(stationNames);
  topLayout->addWidget(fromTime);
  topLayout->addWidget(toTime);
  topLayout->addLayout(buttonLayout);
}

void ListDialog::showAll(){
  this->show();
}

void ListDialog::hideAll(){
  this->hide();
}

void ListDialog::applyHideClicked(){
  emit ListHide();
  emit ListApply();
}

QString ListDialog::getStart() {
    return fromTime->isoTime().cStr();
}

QString ListDialog::getWeatherElement() {
  return weatherElement;
}

void ListDialog::chooseParameters(const QString& str) {
  weatherElement = str;
 }

QString ListDialog::getEnd() {
  return toTime->isoTime().cStr();
} 


void ListDialog::appendStatInListbox(QString station) {
  stationNames->insertItem(station);
}

void ListDialog::removeStatFromListbox(QString station) {
  int rind = -1;
  for (  int ind = 0; ind < stationNames->numRows(); ind++ ) {
    if ( stationNames->text(ind) == station ) {
      rind = ind;
    }
  }
  if ( rind >= 0 )
    stationNames->removeItem(rind);
}

void ListDialog::removeAllStatFromListbox() {
  int nuRo = stationNames->count();
  for (  int ind = 0; ind < nuRo; ind++ ) {
    stationNames->removeItem(0);
  }
}

void ListDialog::twiCheck() {
  if (twiType->isChecked() ) {
    prcType->setChecked(FALSE);
    aprType->setChecked(FALSE);
    winType->setChecked(FALSE);
    visType->setChecked(FALSE);
    marType->setChecked(FALSE);
  }
}

void ListDialog::prcCheck() {
  if (prcType->isChecked() ) {
    twiType->setChecked(FALSE);
    aprType->setChecked(FALSE);
    winType->setChecked(FALSE);
    visType->setChecked(FALSE);
    marType->setChecked(FALSE);
  }
}

void ListDialog::aprCheck() {
  if (aprType->isChecked() ) {
    prcType->setChecked(FALSE);
    twiType->setChecked(FALSE);
    visType->setChecked(FALSE);
    marType->setChecked(FALSE);
  }
}

void ListDialog::winCheck() {
  if (winType->isChecked() ) {
    prcType->setChecked(FALSE);
    twiType->setChecked(FALSE);
    visType->setChecked(FALSE);
    marType->setChecked(FALSE);
  }
}

void ListDialog::visCheck() {
  if (visType->isChecked() ) {
    prcType->setChecked(FALSE);
    aprType->setChecked(FALSE);
    winType->setChecked(FALSE);
    twiType->setChecked(FALSE);
    marType->setChecked(FALSE);
  }
}

void ListDialog::marCheck() {
  if (marType->isChecked() ) {
    prcType->setChecked(FALSE);
    aprType->setChecked(FALSE);
    winType->setChecked(FALSE);
    twiType->setChecked(FALSE);
    visType->setChecked(FALSE);
  }
}

void ListDialog::otwiCheck() {
  aaType->setChecked(FALSE);
  afType->setChecked(FALSE);
  alType->setChecked(FALSE);
  avType->setChecked(FALSE);
  aoType->setChecked(FALSE);
  aeType->setChecked(FALSE);
  mvType->setChecked(FALSE);
  mpType->setChecked(FALSE);
  mmType->setChecked(FALSE);
  msType->setChecked(FALSE);
  fmType->setChecked(FALSE);
  ndType->setChecked(FALSE);
  noType->setChecked(FALSE);
  piType->setChecked(FALSE);
  ptType->setChecked(FALSE);
  vsType->setChecked(FALSE);
  vkType->setChecked(FALSE);
  allType->setChecked(FALSE);
  if ( twiType->isChecked() ) {
    aaType->setChecked(TRUE);
    afType->setChecked(TRUE);
    alType->setChecked(TRUE);
    avType->setChecked(TRUE);
    aoType->setChecked(TRUE);
    aeType->setChecked(TRUE);
    mvType->setChecked(TRUE);
    mpType->setChecked(TRUE);
    mmType->setChecked(TRUE);
    msType->setChecked(TRUE);
    nsType->setChecked(TRUE);
    fmType->setChecked(TRUE);
    ptType->setChecked(TRUE);
    vsType->setChecked(TRUE);
    vkType->setChecked(TRUE);
    vmType->setChecked(TRUE);
  }
}

void ListDialog::oprcCheck() {
  aaType->setChecked(FALSE);
  afType->setChecked(FALSE);
  alType->setChecked(FALSE);
  avType->setChecked(FALSE);
  aoType->setChecked(FALSE);
  aeType->setChecked(FALSE);
  mvType->setChecked(FALSE);
  mpType->setChecked(FALSE);
  mmType->setChecked(FALSE);
  msType->setChecked(FALSE);
  fmType->setChecked(FALSE);
  nsType->setChecked(FALSE);
  ndType->setChecked(FALSE);
  noType->setChecked(FALSE);
  piType->setChecked(FALSE);
  ptType->setChecked(FALSE);
  vsType->setChecked(FALSE);
  vkType->setChecked(FALSE);
  vmType->setChecked(FALSE);
  allType->setChecked(FALSE);
  if ( prcType->isChecked() ) {
    aaType->setChecked(TRUE);
    alType->setChecked(TRUE);
    aoType->setChecked(TRUE);
    nsType->setChecked(TRUE);
    ndType->setChecked(TRUE);
    noType->setChecked(TRUE);
    piType->setChecked(TRUE);
    vsType->setChecked(TRUE);
    vkType->setChecked(TRUE);
    vmType->setChecked(TRUE);
  }
}

void ListDialog::oaprCheck() {
  aaType->setChecked(FALSE);
  afType->setChecked(FALSE);
  alType->setChecked(FALSE);
  avType->setChecked(FALSE);
  aoType->setChecked(FALSE);
  aeType->setChecked(FALSE);
  mvType->setChecked(FALSE);
  mpType->setChecked(FALSE);
  mmType->setChecked(FALSE);
  msType->setChecked(FALSE);
  fmType->setChecked(FALSE);
  nsType->setChecked(FALSE);
  ndType->setChecked(FALSE);
  noType->setChecked(FALSE);
  piType->setChecked(FALSE);
  ptType->setChecked(FALSE);
  vsType->setChecked(FALSE);
  vkType->setChecked(FALSE);
  vmType->setChecked(FALSE);
  allType->setChecked(FALSE);
  if ( aprType->isChecked() ) {
    aaType->setChecked(TRUE);
    afType->setChecked(TRUE);
    aeType->setChecked(TRUE);
    mvType->setChecked(TRUE);
    mpType->setChecked(TRUE);
    mmType->setChecked(TRUE);
    msType->setChecked(TRUE);
    vsType->setChecked(TRUE);
  }
  if ( winType->isChecked() ) {
    aaType->setChecked(TRUE);
    afType->setChecked(TRUE);
    alType->setChecked(TRUE);
    avType->setChecked(TRUE);
    aoType->setChecked(TRUE);
    aeType->setChecked(TRUE);
    mvType->setChecked(TRUE);
    mpType->setChecked(TRUE);
    mmType->setChecked(TRUE);
    msType->setChecked(TRUE);
    fmType->setChecked(TRUE);
    vsType->setChecked(TRUE);
    vkType->setChecked(TRUE);
  }
}

void ListDialog::owinCheck() {
  aaType->setChecked(FALSE);
  afType->setChecked(FALSE);
  alType->setChecked(FALSE);
  avType->setChecked(FALSE);
  aoType->setChecked(FALSE);
  aeType->setChecked(FALSE);
  mvType->setChecked(FALSE);
  mpType->setChecked(FALSE);
  mmType->setChecked(FALSE);
  msType->setChecked(FALSE);
  fmType->setChecked(FALSE);
  nsType->setChecked(FALSE);
  ndType->setChecked(FALSE);
  noType->setChecked(FALSE);
  piType->setChecked(FALSE);
  ptType->setChecked(FALSE);
  vsType->setChecked(FALSE);
  vkType->setChecked(FALSE);
  vmType->setChecked(FALSE);
  allType->setChecked(FALSE);
  if ( aprType->isChecked() ) {
    aaType->setChecked(TRUE);
    afType->setChecked(TRUE);
    aeType->setChecked(TRUE);
    mvType->setChecked(TRUE);
    mpType->setChecked(TRUE);
    mmType->setChecked(TRUE);
    msType->setChecked(TRUE);
    vsType->setChecked(TRUE);
  }
  if ( winType->isChecked() ) {
    aaType->setChecked(TRUE);
    afType->setChecked(TRUE);
    alType->setChecked(TRUE);
    avType->setChecked(TRUE);
    aoType->setChecked(TRUE);
    aeType->setChecked(TRUE);
    mvType->setChecked(TRUE);
    mpType->setChecked(TRUE);
    mmType->setChecked(TRUE);
    msType->setChecked(TRUE);
    fmType->setChecked(TRUE);
    vsType->setChecked(TRUE);
    vkType->setChecked(TRUE);
  }
}

void ListDialog::ovisCheck() {
  aaType->setChecked(FALSE);
  afType->setChecked(FALSE);
  alType->setChecked(FALSE);
  avType->setChecked(FALSE);
  aoType->setChecked(FALSE);
  aeType->setChecked(FALSE);
  mvType->setChecked(FALSE);
  mpType->setChecked(FALSE);
  mmType->setChecked(FALSE);
  msType->setChecked(FALSE);
  fmType->setChecked(FALSE);
  nsType->setChecked(FALSE);
  ndType->setChecked(FALSE);
  noType->setChecked(FALSE);
  piType->setChecked(FALSE);
  ptType->setChecked(FALSE);
  vsType->setChecked(FALSE);
  vkType->setChecked(FALSE);
  vmType->setChecked(FALSE);
  allType->setChecked(FALSE);
  if ( visType->isChecked() ) {
    mvType->setChecked(TRUE);
    mpType->setChecked(TRUE);
    mmType->setChecked(TRUE);
    msType->setChecked(TRUE);
    fmType->setChecked(TRUE);
    nsType->setChecked(TRUE);
    ndType->setChecked(TRUE);
    noType->setChecked(TRUE);
    vsType->setChecked(TRUE);
    vkType->setChecked(TRUE);
    vmType->setChecked(TRUE);
  }
}

void ListDialog::omarCheck() {
  aaType->setChecked(FALSE);
  afType->setChecked(FALSE);
  alType->setChecked(FALSE);
  avType->setChecked(FALSE);
  aoType->setChecked(FALSE);
  aeType->setChecked(FALSE);
  mvType->setChecked(FALSE);
  mpType->setChecked(FALSE);
  mmType->setChecked(FALSE);
  msType->setChecked(FALSE);
  fmType->setChecked(FALSE);
  nsType->setChecked(FALSE);
  ndType->setChecked(FALSE);
  noType->setChecked(FALSE);
  piType->setChecked(FALSE);
  ptType->setChecked(FALSE);
  vsType->setChecked(FALSE);
  vkType->setChecked(FALSE);
  vmType->setChecked(FALSE);
  allType->setChecked(FALSE);
  if ( marType->isChecked() ) {
    mvType->setChecked(TRUE);
    mpType->setChecked(TRUE);
    mmType->setChecked(TRUE);
    msType->setChecked(TRUE);
  }
}

void ListDialog::ausCheck() {
  if (ausReg->isChecked() ) {
    vesReg->setChecked(FALSE);
    troReg->setChecked(FALSE);
    norReg->setChecked(FALSE);
    webReg->setChecked(FALSE);
    priReg->setChecked(FALSE);
  }
}

void ListDialog::oausCheck() {
  oslCoun->setChecked(FALSE);
  akeCoun->setChecked(FALSE);
  ostCoun->setChecked(FALSE);
  hedCoun->setChecked(FALSE);
  oppCoun->setChecked(FALSE);
  busCoun->setChecked(FALSE);
  vefCoun->setChecked(FALSE);
  telCoun->setChecked(FALSE);
  ausCoun->setChecked(FALSE);
  veaCoun->setChecked(FALSE);
  rogCoun->setChecked(FALSE);
  horCoun->setChecked(FALSE);
  sogCoun->setChecked(FALSE);
  morCoun->setChecked(FALSE);
  sorCoun->setChecked(FALSE);
  ntrCoun->setChecked(FALSE);
  norCoun->setChecked(FALSE);
  troCoun->setChecked(FALSE);
  finCoun->setChecked(FALSE);
  svaCoun->setChecked(FALSE);
  allCoun->setChecked(FALSE);
  if ( ausReg->isChecked() ) {
    oslCoun->setChecked(TRUE);
    akeCoun->setChecked(TRUE);
    ostCoun->setChecked(TRUE);
    hedCoun->setChecked(TRUE);
    oppCoun->setChecked(TRUE);
    busCoun->setChecked(TRUE);
    vefCoun->setChecked(TRUE);
    telCoun->setChecked(TRUE);
    ausCoun->setChecked(TRUE);
  }
}

void ListDialog::vesCheck() {
  if (vesReg->isChecked() ) {
    ausReg->setChecked(FALSE);
    troReg->setChecked(FALSE);
    norReg->setChecked(FALSE);
    webReg->setChecked(FALSE);
    priReg->setChecked(FALSE);
  }
}

void ListDialog::ovesCheck() {
  oslCoun->setChecked(FALSE);
  akeCoun->setChecked(FALSE);
  ostCoun->setChecked(FALSE);
  hedCoun->setChecked(FALSE);
  oppCoun->setChecked(FALSE);
  busCoun->setChecked(FALSE);
  vefCoun->setChecked(FALSE);
  telCoun->setChecked(FALSE);
  ausCoun->setChecked(FALSE);
  veaCoun->setChecked(FALSE);
  rogCoun->setChecked(FALSE);
  horCoun->setChecked(FALSE);
  sogCoun->setChecked(FALSE);
  morCoun->setChecked(FALSE);
  sorCoun->setChecked(FALSE);
  ntrCoun->setChecked(FALSE);
  norCoun->setChecked(FALSE);
  troCoun->setChecked(FALSE);
  finCoun->setChecked(FALSE);
  svaCoun->setChecked(FALSE);
  allCoun->setChecked(FALSE);
  if ( vesReg->isChecked() ) {
    veaCoun->setChecked(TRUE);
    rogCoun->setChecked(TRUE);
    horCoun->setChecked(TRUE);
    sogCoun->setChecked(TRUE);
    morCoun->setChecked(TRUE);
  }
}

void ListDialog::troCheck() {
  if (troReg->isChecked() ) {
    ausReg->setChecked(FALSE);
    vesReg->setChecked(FALSE);
    norReg->setChecked(FALSE);
    webReg->setChecked(FALSE);
    priReg->setChecked(FALSE);
  }
}

void ListDialog::otroCheck() {
  oslCoun->setChecked(FALSE);
  akeCoun->setChecked(FALSE);
  ostCoun->setChecked(FALSE);
  hedCoun->setChecked(FALSE);
  oppCoun->setChecked(FALSE);
  busCoun->setChecked(FALSE);
  vefCoun->setChecked(FALSE);
  telCoun->setChecked(FALSE);
  ausCoun->setChecked(FALSE);
  veaCoun->setChecked(FALSE);
  rogCoun->setChecked(FALSE);
  horCoun->setChecked(FALSE);
  sogCoun->setChecked(FALSE);
  morCoun->setChecked(FALSE);
  sorCoun->setChecked(FALSE);
  ntrCoun->setChecked(FALSE);
  norCoun->setChecked(FALSE);
  troCoun->setChecked(FALSE);
  finCoun->setChecked(FALSE);
  svaCoun->setChecked(FALSE);
  allCoun->setChecked(FALSE);
  if ( troReg->isChecked() ) {
    sorCoun->setChecked(TRUE);
    ntrCoun->setChecked(TRUE);
  }
}

void ListDialog::norCheck() {
  if (norReg->isChecked() ) {
    vesReg->setChecked(FALSE);
    troReg->setChecked(FALSE);
    ausReg->setChecked(FALSE);
    webReg->setChecked(FALSE);
    priReg->setChecked(FALSE);
  }
}

void ListDialog::onorCheck() {
  oslCoun->setChecked(FALSE);
  akeCoun->setChecked(FALSE);
  ostCoun->setChecked(FALSE);
  hedCoun->setChecked(FALSE);
  oppCoun->setChecked(FALSE);
  busCoun->setChecked(FALSE);
  vefCoun->setChecked(FALSE);
  telCoun->setChecked(FALSE);
  ausCoun->setChecked(FALSE);
  veaCoun->setChecked(FALSE);
  rogCoun->setChecked(FALSE);
  horCoun->setChecked(FALSE);
  sogCoun->setChecked(FALSE);
  morCoun->setChecked(FALSE);
  sorCoun->setChecked(FALSE);
  ntrCoun->setChecked(FALSE);
  norCoun->setChecked(FALSE);
  troCoun->setChecked(FALSE);
  finCoun->setChecked(FALSE);
  svaCoun->setChecked(FALSE);
  allCoun->setChecked(FALSE);
  if ( norReg->isChecked() ) {
    norCoun->setChecked(TRUE);
    troCoun->setChecked(TRUE);
    finCoun->setChecked(TRUE);
    svaCoun->setChecked(TRUE);
  }
}

void ListDialog::webCheck() {
  if (webReg->isChecked() ) {
    allType->setChecked(TRUE);
  }
}

void ListDialog::owebCheck() {
  oslCoun->setChecked(FALSE);
  akeCoun->setChecked(FALSE);
  ostCoun->setChecked(FALSE);
  hedCoun->setChecked(FALSE);
  oppCoun->setChecked(FALSE);
  busCoun->setChecked(FALSE);
  vefCoun->setChecked(FALSE);
  telCoun->setChecked(FALSE);
  ausCoun->setChecked(FALSE);
  veaCoun->setChecked(FALSE);
  rogCoun->setChecked(FALSE);
  horCoun->setChecked(FALSE);
  sogCoun->setChecked(FALSE);
  morCoun->setChecked(FALSE);
  sorCoun->setChecked(FALSE);
  ntrCoun->setChecked(FALSE);
  norCoun->setChecked(FALSE);
  troCoun->setChecked(FALSE);
  finCoun->setChecked(FALSE);
  svaCoun->setChecked(FALSE);
  allCoun->setChecked(FALSE);
}
void ListDialog::priCheck() {
  if (priReg->isChecked() ) {
    allType->setChecked(TRUE);
  }
}

void ListDialog::opriCheck() {
  oslCoun->setChecked(FALSE);
  akeCoun->setChecked(FALSE);
  ostCoun->setChecked(FALSE);
  hedCoun->setChecked(FALSE);
  oppCoun->setChecked(FALSE);
  busCoun->setChecked(FALSE);
  vefCoun->setChecked(FALSE);
  telCoun->setChecked(FALSE);
  ausCoun->setChecked(FALSE);
  veaCoun->setChecked(FALSE);
  rogCoun->setChecked(FALSE);
  horCoun->setChecked(FALSE);
  sogCoun->setChecked(FALSE);
  morCoun->setChecked(FALSE);
  sorCoun->setChecked(FALSE);
  ntrCoun->setChecked(FALSE);
  norCoun->setChecked(FALSE);
  troCoun->setChecked(FALSE);
  finCoun->setChecked(FALSE);
  svaCoun->setChecked(FALSE);
  allCoun->setChecked(FALSE);
}
void ListDialog::allCounCheck() {
  if ( allCoun->isChecked() ) {
    oslCoun->setChecked(FALSE);
    akeCoun->setChecked(FALSE);
    ostCoun->setChecked(FALSE);
    hedCoun->setChecked(FALSE);
    oppCoun->setChecked(FALSE);
    busCoun->setChecked(FALSE);
    vefCoun->setChecked(FALSE);
    telCoun->setChecked(FALSE);
    ausCoun->setChecked(FALSE);
    veaCoun->setChecked(FALSE);
    rogCoun->setChecked(FALSE);
    horCoun->setChecked(FALSE);
    sogCoun->setChecked(FALSE);
    morCoun->setChecked(FALSE);
    sorCoun->setChecked(FALSE);
    ntrCoun->setChecked(FALSE);
    norCoun->setChecked(FALSE);
    troCoun->setChecked(FALSE);
    finCoun->setChecked(FALSE);
    svaCoun->setChecked(FALSE);
    vesReg->setChecked(FALSE);
    troReg->setChecked(FALSE);
    ausReg->setChecked(FALSE);
    norReg->setChecked(FALSE);
    priReg->setChecked(FALSE);
  }
}

void ListDialog::allCounUnCheck() {
  if ( oslCoun->isChecked() ||
  oslCoun->isChecked() ||
  akeCoun->isChecked() ||
  ostCoun->isChecked() ||
  hedCoun->isChecked() ||
  oppCoun->isChecked() ||
  busCoun->isChecked() ||
  vefCoun->isChecked() ||
  telCoun->isChecked() ||
  ausCoun->isChecked() ||
  veaCoun->isChecked() ||
  rogCoun->isChecked() ||
  horCoun->isChecked() ||
  sogCoun->isChecked() ||
  morCoun->isChecked() ||
  sorCoun->isChecked() ||
  ntrCoun->isChecked() ||
  norCoun->isChecked() ||
  svaCoun->isChecked() ||
  finCoun->isChecked() ) {
    allCoun->setChecked(FALSE);
  }
}

StationTable::StationTable(QStringList selStatNum,
			   QStringList selStatName,
			   QStringList selStatHoh,
			   QStringList selStatType,
			   QStringList selStatFylke,
			   QStringList selStatKommune,
			   QStringList selStatWeb,
			   QStringList selStatPri,
			   int noStat,
			   bool aa,
			   bool af,
			   bool al, 
			   bool av,
			   bool ao,
			   bool ae, 
			   bool mv, 
			   bool mp, 
			   bool mm, 
			   bool ms, 
			   bool fm, 
			   bool ns,
			   bool nd,
			   bool no, 
			   bool pi, 
			   bool pt, 
			   bool vs, 
			   bool vk, 
			   bool vm, 
			   bool all, 
			   bool osl,
			   bool ake,
			   bool ost, 
			   bool hed,
			   bool opp,
			   bool bus, 
			   bool vef, 
			   bool tel, 
			   bool aus, 
			   bool vea, 
			   bool rog, 
			   bool hor,
			   bool sog,
			   bool mor, 
			   bool sor, 
			   bool ntr, 
			   bool nor, 
			   bool tro, 
			   bool fin,
			   bool sva,
			   bool allc,
			   bool web, 
			   bool pri, 
			   int noInfo,
			   ObsTypeList* otpList,
			   QWidget* parent)
    : Q3Table( 3000, noInfo, parent)
{

  setCaption("Stasjoner");
  setSorting( TRUE );
  setGeometry(10,100,800,600);

  horizontalHeader()->setLabel( 0, tr( "Stnr" ) );
  horizontalHeader()->setLabel( 1, tr( "Navn" ) );
  horizontalHeader()->setLabel( 2, tr( "HOH" ) );
  horizontalHeader()->setLabel( 3, tr( "Type" ) );
  horizontalHeader()->setLabel( 4, tr( "Fylke" ) );
  horizontalHeader()->setLabel( 5, tr( "Kommune" ) );
  horizontalHeader()->setLabel( 6, tr( "Pri" ) );
  int stInd = 0;
  for ( int i = 0; i < noStat; i++) {
    QString strStnr      = selStatNum.at(i);
    QString testName     = selStatName.at(i);
    QString strStName    = selStatName.at(i);
    QString strStHoh     = selStatHoh.at(i);
    QString strStType    = selStatType.at(i);
    QString strStFylke   = selStatFylke.at(i);
    QString strStKommune = selStatKommune.at(i);
    bool webStat = selStatWeb.at(i) != "    ";
    bool priStat = (selStatPri.at(i)).left(3) == "PRI";
    QString prty =  (selStatPri.at(i)).right(1);
    int stano = strStnr.toInt();
    ObsTypeList::iterator oit = otpList->begin();
    bool foundStat = false;
    for ( ; oit != otpList->end(); oit++) {   
      TypeList::iterator tit = oit->begin();
      if ( stano == (*tit) ) {
	foundStat = true;
	break;
      }
    }
    if ( !foundStat ) {
      continue;
    }
    if ( ! (allc == TRUE ||
	    (osl == TRUE && strStFylke == "OSLO") ||
	    (ake == TRUE && strStFylke == "AKERSHUS") ||
	    (ost == TRUE && strStFylke == "ØSTFOLD") ||
	    (hed == TRUE && strStFylke == "HEDMARK") ||
	    (opp == TRUE && strStFylke == "OPPLAND") ||
	    (bus == TRUE && strStFylke == "BUSKERUD") ||
	    (vef == TRUE && strStFylke == "VESTFOLD") ||
	    (tel == TRUE && strStFylke == "TELEMARK") ||
	    (aus == TRUE && strStFylke == "AUST-AGDER") ||
	    (vea == TRUE && strStFylke == "VEST-AGDER") ||
	    (rog == TRUE && strStFylke == "ROGALAND") ||
	    (hor == TRUE && strStFylke == "HORDALAND") ||
	    (sog == TRUE && strStFylke == "SOGN OG FJORDANE") ||
	    (mor == TRUE && strStFylke == "MØRE OG ROMSDAL") ||
	    (sor == TRUE && strStFylke == "SØR-TRØNDELAG") ||
	    (ntr == TRUE && strStFylke == "NORD-TRØNDELAG") ||
	    (nor == TRUE && strStFylke == "NORDLAND") ||
	    (tro == TRUE && strStFylke == "TROMS") ||
	    (fin == TRUE && strStFylke == "FINNMARK") ||
	    (sva == TRUE && strStFylke == "ISHAVET") ||
       	    (webStat && web) || (priStat && pri) ))
      continue;
    QString strEnv;
    if ( (aa == TRUE && ((strStType == "8" && (findInTypes(oit, 3)  || findInTypes(oit, 311))) || findInTypes(oit, 330) || findInTypes(oit, 342))) ) strEnv += "AA";
    if ( (af == TRUE && strStType == "1" && findInTypes(oit, 311)) )  strEnv += "AF";
    if ( (al == TRUE && strStType == "2" && findInTypes(oit, 3)) ) strEnv += "AL"; 
    if ( (av == TRUE && strStType == "12" && findInTypes(oit, 3)) )  strEnv += "AV";
    if ( (ao == TRUE && findInTypes(oit, 410)) )  strEnv += "AO";
    if ( (mv == TRUE && strStType == "7" && findInTypes(oit, 11)) ) strEnv += "MV";
    if ( (mp == TRUE && strStType == "5" && findInTypes(oit, 11)) ) strEnv += "MP";
    if ( (mm == TRUE && strStType == "4" && findInTypes(oit, 11)) ) strEnv += "MM";
    if ( (ms == TRUE && strStType == "6" && findInTypes(oit, 11)) ) strEnv += "MS";
    if ( (pi == TRUE && (findInTypes(oit, 4) || findInTypes(oit, 404))) ) strEnv += "P";
    if ( (pt == TRUE && (findInTypes(oit, 4) || findInTypes(oit, 404))) ) strEnv += "PT";
    if ( (ns == TRUE && findInTypes(oit, 302)) )  strEnv += "NS";
    if ( (nd == TRUE && strStType == "9" && findInTypes(oit, 402)) ) strEnv += "ND";
    if ( (no == TRUE && strStType == "10" && findInTypes(oit, 402)) ) strEnv += "NO";
    if ( (vs == TRUE && (findInTypes(oit, 1) || findInTypes(oit, 6) || findInTypes(oit, 312))) ) strEnv += "VS";
    if ( (vk == TRUE && strStType == "3" && findInTypes(oit, 412)) ) strEnv += "VK";
    if ( (vm == TRUE && (findInTypes(oit, 306) || findInTypes(oit, 308))) ) strEnv += "VM";
    if ( (fm == TRUE && (findInTypes(oit, 2))) ) strEnv += "FM";
    if (all == TRUE )
      strEnv = getEnvironment(strStType, oit);
    if ( !strEnv.isEmpty() ) {
      StTableItem* stNum = new StTableItem(this, Q3TableItem::Never, strStnr);
      setItem(stInd, 0, stNum);
      StTableItem* stName = new StTableItem(this, Q3TableItem::Never, strStName);
      setItem(stInd, 1, stName);
      StTableItem* stHoh = new StTableItem(this, Q3TableItem::Never, strStHoh);
      setItem(stInd, 2, stHoh);
      StTableItem* stType = new StTableItem(this, Q3TableItem::Never, strEnv);
      setItem(stInd, 3, stType);
      StTableItem* stFylke = new StTableItem(this, Q3TableItem::Never, strStFylke);
      setItem(stInd, 4, stFylke);
      StTableItem* stKommune = new StTableItem(this, 
					       Q3TableItem::Never, 
					       strStKommune);
      setItem(stInd, 5, stKommune);
      StTableItem* stPrior = new StTableItem(this, 
					       Q3TableItem::Never, 
					       prty);
      setItem(stInd, 6, stPrior);
      stInd++;
    }
  }
  setNumRows(stInd);
 
  adjustColumn( 0 );
  adjustColumn( 1 );
  adjustColumn( 2 );
  adjustColumn( 3 );
  adjustColumn( 4 );
  adjustColumn( 5 );
  adjustColumn( 6 );
  if ( pri) 
    sortColumn(6, TRUE, TRUE);
  else
    hideColumn(6);
}

bool StationTable::findInTypes(ObsTypeList::iterator tList, int type) {
  TypeList::iterator tpind = std::find(tList->begin(), tList->end(), type);
  if ( tpind == tList->end() )
    return false;
  else
    return true;
}

QString StationTable::getEnvironment(QString strStType, ObsTypeList::iterator oit) {
  QString env;
  if ( strStType == "1" && findInTypes(oit, 311) )
    env = "AF";
  else if ( strStType == "2" && findInTypes(oit, 3) )
    env = "AL"; 
  else if ( strStType == "4" && findInTypes(oit, 11) )
    env = "MM"; 
  else if ( strStType == "5" && findInTypes(oit, 11) )
    env = "MP"; 
  else if ( strStType == "6" && findInTypes(oit, 11) )
    env = "MS"; 
  else if ( strStType == "7" && findInTypes(oit, 11) )
    env = "MV"; 
  else if ( (strStType == "8" && (findInTypes(oit, 3)  || findInTypes(oit, 311))) || findInTypes(oit, 330) || findInTypes(oit, 342) )
    env = "AA"; 
  else if ( strStType == "9" && findInTypes(oit, 402) )
    env = "ND"; 
  else if ( strStType == "10" && findInTypes(oit, 402) )
    env = "NO"; 
  else if ( findInTypes(oit, 302) )
    env = "NS"; 
  else if ( findInTypes(oit, 410) )
    env = "AO"; 
  else if ( findInTypes(oit, 4) || findInTypes(oit, 404) )
    env = "P,PT"; 
  else if ( findInTypes(oit, 2) )
    env = "FM"; 
  else if ( findInTypes(oit, 1) || findInTypes(oit, 6) || findInTypes(oit, 312) )
    env = "VS"; 
  else if ( findInTypes(oit, 306) || findInTypes(oit, 308) )
    env = "VM"; 
  else if ( strStType == "11" )
    env = "TURISTFORENING";
  else if ( strStType == "12" && findInTypes(oit, 3) )
    env = "AV";
  else if ( findInTypes(oit, 412) )
    env = "VK"; 
  return env;
}


void StationTable::sortColumn( int col, bool ascending, bool /*wholeRows*/ ) {
    Q3Table::sortColumn( col, ascending, TRUE );
}

StationSelection::StationSelection(QStringList listStatNum, 
				   QStringList listStatName, 
				   QStringList listStatHoh, 
				   QStringList listStatType, 
				   QStringList listStatFylke, 
				   QStringList listStatKommune, 
				   QStringList listStatWeb, 
				   QStringList listStatPri, 
				   int noStat,
				   bool aa,
				   bool af,
				   bool al, 
				   bool av,
				   bool ao,
				   bool ae, 
				   bool mv, 
				   bool mp, 
				   bool mm, 
				   bool ms, 
				   bool fm, 
				   bool ns,
				   bool nd,
				   bool no, 
				   bool pi, 
				   bool pt, 
				   bool vs, 
				   bool vk, 
				   bool vm,
				   bool all, 
				   bool osl,
				   bool ake,
				   bool ost, 
				   bool hed,
				   bool opp,
				   bool bus, 
				   bool vef, 
				   bool tel, 
				   bool aus, 
				   bool vea, 
				   bool rog, 
				   bool hor,
				   bool sog,
				   bool mor, 
				   bool sor, 
				   bool ntr, 
				   bool nor, 
				   bool tro, 
				   bool fin,
				   bool sva,
				   bool allc,
				   bool web, 
				   bool pri, 
				   int noInfo,
				   ObsTypeList* otpList) : QWidget() {

  setGeometry(0,0,870,700);
  selectionOK = new QPushButton("Lukk", this);
  selectionOK->setGeometry(50,10,130,30);
  connect(selectionOK, SIGNAL(clicked()),SLOT(listSelectedStations()));
  selectAllStations = new QPushButton("Velg alle stasjoner", this);
  selectAllStations->setGeometry(200,10,130,30);
  connect(selectAllStations, SIGNAL(clicked()),SLOT(showAllStations()));
  stationTable = new StationTable(listStatNum, 
				  listStatName, 
				  listStatHoh, 
				  listStatType, 
				  listStatFylke, 
				  listStatKommune, 
				  listStatWeb, 
				  listStatPri, 
				  noStat, 
				  aa,
				  af,
				  al, 
				  av,
				  ao,
				  ae, 
				  mv, 
				  mp, 
				  mm, 
				  ms, 
				  fm, 
				  ns,
				  nd,
				  no, 
				  pi, 
				  pt, 
				  vs, 
				  vk, 
				  vm, 
				  all, 
				  osl,
				  ake,
				  ost, 
				  hed,
				  opp,
				  bus, 
				  vef, 
				  tel, 
				  aus, 
				  vea, 
				  rog, 
				  hor,
				  sog,
				  mor, 
				  sor, 
				  ntr, 
				  nor, 
				  tro, 
				  fin,
				  sva,
				  allc,
				  web,
				  pri,
				  noInfo,
				  otpList, 
				  this);
  connect( stationTable,SIGNAL(currentChanged(int, int)),
	   SLOT(tableCellClicked(int, int)));  
}

void StationSelection::tableCellClicked() {
}
void StationSelection::tableCellClicked(int row, 
					int col, 
					int button, 
					const QPoint& mousePos) {
  stationTable->selectRow(row);
  showSelectedStation(row, 0);
}

void StationSelection::tableCellClicked(int row, int col) {
  stationTable->selectRow(row);
  showSelectedStation(row, 0);
}

void StationSelection::showSelectedStation(int row, int col) {
  Q3TableItem* tStationNumber = stationTable->item( row, 0);
  Q3TableItem* tStationName = stationTable->item( row, 1);
  QString station = tStationNumber->text() + "  " + tStationName->text();
  int rem = stlist.remove(station);
  if ( rem == 0 ) {
    stlist.append(station);
    emit stationAppended(station);
  }
  else {
    emit stationRemoved(station);
  }
}

void StationSelection::showAllStations() {
  for (  int row = 0; row < stationTable->numRows(); row++ ) {
    Q3TableItem* tStationNumber = stationTable->item( row, 0);
    Q3TableItem* tStationName = stationTable->item( row, 1);
    QString station = tStationNumber->text() + "  " + tStationName->text();
    int rem = stlist.remove(station);
    if ( rem == 0 ) {
      stlist.append(station);
      emit stationAppended(station);
    }
  }
}

void StationSelection::listSelectedStations() {
  emit stationsSelected(stlist);
  this->hide();
}

QString StTableItem::key() const {
  QString item;
  if ( col() == 0 || col() == 2 ) {
    item.sprintf("%08d",text().toInt());
  }
  else {
    item = text();
  }
  return item;
}

