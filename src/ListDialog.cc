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

#include "HideApplyBox.hh"
#include "MiDateTimeEdit.hh"
#include "mi_foreach.hh"
#include "timeutil.hh"

#include <Qt3Support/Q3HBoxLayout>
#include <QtGui/QLabel>
#include <Qt3Support/Q3GridLayout>
#include <Qt3Support/Q3VBoxLayout>

#include <algorithm>

namespace /* anonymous */ {
struct stationtype_t {
    const char* name;
    int gridX, gridY;
};
const int NSTATIONTYPES = 19;
const stationtype_t stationTypes[NSTATIONTYPES] = {
  { "AA", 0, 0 },
  { "AF", 1, 0 },
  { "AL", 2, 0 },
  { "AV", 3, 0 },
  { "AO", 4, 0 },
  { "AE", 5, 0 },
  { "MV", 0, 1 },
  { "MP", 1, 1 },
  { "MM", 2, 1 },
  { "MS", 3, 1 },
  { "FM", 4, 3 },
  { "NS", 0, 2 },
  { "ND", 1, 2 },
  { "NO", 2, 2 },
  { "P",  4, 2 },
  { "PT", 5, 2 },
  { "VS", 0, 3 },
  { "VK", 1, 3 },
  { "VM", 2, 3 }
};

#if 0
const int NCOUNTIES = 20;
const char* counties[NCOUNTIES] =  {
  "Oslo", "Akershus", "Østfold", "Hedmark", "Oppland", "Buskerud", "Vestfold", "Telemark",
  "Aust-Agder", "Vest-Agder", "Rogaland", "Hordaland", "Sogn og Fjordane", "Møre og Romsdal",
  "Sør-Trøndelag", "Nord-Trøndelag", "Nordland", "Troms", "Finnmark", "Ishavet"
};
const char* countiesU[NCOUNTIES] =  {
    "OSLO", "AKERSHUS", "ØSTFOLD", "HEDMARK", "OPPLAND", "BUSKERUD", "VESTFOLD", "TELEMARK",
    "AUST-AGDER", "VEST-AGDER", "ROGALAND", "HORDALAND", "SOGN OG FJORDANE", "MØRE OG ROMSDAL",
    "SØR-TRØNDELAG", "NORD-TRØNDELAG", "NORDLAND", "TROMS", "FINNMARK", "ISHAVET"
};
#endif
} // anonymous namespace

void ItemCheckBox::clicked()
{
    emit clicked(mItem);
}

ListDialog::ListDialog(QWidget* parent)
  : QDialog(parent)
{
  setCaption(tr("Datautvalg HQC"));

  // Create a button group for control type

  Q3ButtonGroup *ctrlTyp = new Q3ButtonGroup( 1,
					  Qt::Horizontal,
					  tr("Kontrolltype"), this);
  Q3GridLayout* controlLayout = new Q3GridLayout(ctrlTyp->layout());

  // insert checkbuttons for control type selection
  twiType = new QCheckBox( tr("&Temperatur,fuktighet"), ctrlTyp );
  prcType = new QCheckBox( tr("&Nedbør,snøforhold"), ctrlTyp );
  aprType = new QCheckBox( tr("&Lufttrykk"), ctrlTyp );
  winType = new QCheckBox( tr("&Vind"), ctrlTyp );
  marType = new QCheckBox( tr("&Maritime parametere"), ctrlTyp );
  visType = new QCheckBox( tr("V&isuelle parametere"), ctrlTyp );

  controlLayout->addWidget(twiType, 0, 0);
  controlLayout->addWidget(prcType, 1, 0);
  controlLayout->addWidget(aprType, 2, 0);
  controlLayout->addWidget(winType, 3, 0);
  controlLayout->addWidget(marType, 4, 0);
  controlLayout->addWidget(visType, 5, 0);

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

  // insert checkbuttons for station type selection
  Q3ButtonGroup *stTyp = new Q3ButtonGroup(0, Qt::Horizontal,
					 tr("Stasjonstype"),
					 this);
  Q3GridLayout* statSelLayout = new Q3GridLayout(stTyp->layout());
  for(int i=0; i<NSTATIONTYPES; ++i) {
      const stationtype_t& s = stationTypes[i];
      ItemCheckBox* cb = new ItemCheckBox(s.name, s.name, stTyp);
      statSelLayout->addWidget(cb, s.gridX, s.gridY);
      mStationTypes.push_back(cb);
  }
  allType = new QCheckBox( tr("Alle"), stTyp);
  statSelLayout->addWidget(allType, 5, 3);

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
  allCoun = new QCheckBox( tr("Alle"), stCounty );

   // Create a button group for station location (region)

  Q3ButtonGroup *stRegion = new Q3ButtonGroup( 1,
					  Qt::Horizontal,
					  "Landsdel", this);

  // insert checkbuttons for station location selection
  ausReg = new QCheckBox( "&Østlandet ", stRegion );
  vesReg = new QCheckBox( "V&estlandet", stRegion );
  troReg = new QCheckBox( "T&røndelag ", stRegion );
  norReg = new QCheckBox( "N&ord-Norge", stRegion );
  webReg = new QCheckBox( "S&ynop-stasjoner", stRegion );
  priReg = new QCheckBox( tr("&Prioriterte stasjoner"), stRegion );

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
					  tr("Meldingstyper"), this);

  priTypes = new QRadioButton( tr("Prioriterte typer"), typeGroup );
  typeGroup->insert(priTypes);
  priTypes->setChecked(true);
  allTypes = new QRadioButton( tr("Alle typer"), typeGroup );
  typeGroup->insert(allTypes);

  //Station selection
  stationSelect = new QPushButton(tr("Velg &stasjon"), this);
  stationSelect->setAutoDefault(true);
  stationSelect->setGeometry(10, 110, 400, 30);
  stationSelect->setFont(QFont("Arial", 9));
  connect(stationSelect, SIGNAL(clicked()), this, SIGNAL( selectStation()));

  stationLabel = new QLabel(this);
  stationLabel->setText(tr("Valgte stasjoner"));
  stationLabel->setFont(QFont("Arial", 12));
  stationLabel->setPaletteForegroundColor(Qt::darkBlue);
  stationLabel->setAlignment(Qt::AlignLeft);
  stationNames = new Q3ListBox(this);

  //Time selection
  QDateTime t = timeutil::nowWithMinutes0Seconds0();
  QDateTime f = t.addSecs(-2*24*3600 + 3600*(17-t.time().hour()) + 60*45);
  fromTime = new MiDateTimeEdit(f,this);
  toTime   = new MiDateTimeEdit(t,this);
  fromTime->setDisplayFormat("yyyy-MM-dd hh:mm");
  toTime->setDisplayFormat("yyyy-MM-dd hh:mm");

  connect( fromTime, SIGNAL(dateChanged(const QDate&)),
	   this,   SLOT(  setMinDate(const QDate&)     ));
  connect( fromTime, SIGNAL(timeChanged(const QTime&)),
	   this,   SLOT(  setMinTime(const QTime&)     ));
  connect( fromTime, SIGNAL(dateTimeChanged(const QDateTime&)),
	   this, SIGNAL(fromTimeChanged(const QDateTime&)));

  connect( toTime,  SIGNAL(dateChanged(const QDate&)),
	   this,SLOT(  setMaxDate(const QDate&)     ));
  connect( toTime,  SIGNAL(timeChanged(const QTime&)),
	   this,   SLOT(  setMaxTime(const QTime&)     ));
  connect( toTime,  SIGNAL(dateTimeChanged(const QDateTime&)),
	   this,SIGNAL(toTimeChanged(const QDateTime&)));

  HideApplyBox* hab = new HideApplyBox(this);
  connect(hab, SIGNAL(hide()), this, SIGNAL(ListHide()));
  connect(hab, SIGNAL(apply()), this, SIGNAL(ListApply()));

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

  QLabel* fromLabel = new QLabel(tr("Fra"));
  Q3HBoxLayout* ftimeLayout = new Q3HBoxLayout();
  ftimeLayout->addWidget(fromLabel);
  ftimeLayout->addWidget(fromTime);
  QLabel* toLabel = new QLabel(tr("Til"));
  Q3HBoxLayout* ttimeLayout = new Q3HBoxLayout();
  ttimeLayout->addWidget(toLabel);
  ttimeLayout->addWidget(toTime);

  topLayout = new Q3VBoxLayout(this,10);

  topLayout->addLayout(typeLayout);
  topLayout->addLayout(locationLayout);
  topLayout->addWidget(typeGroup);
  topLayout->addWidget(stationSelect);
  topLayout->addWidget(stationLabel);
  topLayout->addWidget(stationNames);
  topLayout->addLayout(ftimeLayout);
  topLayout->addLayout(ttimeLayout);
  topLayout->addWidget(hab);
}

void ListDialog::setMaxTime(const QTime& maxTime)
{
  fromTime->setMaximumTime(maxTime);
}
void ListDialog::setMinTime(const QTime& minTime)
{
  toTime->setMinimumTime(minTime);
}
void ListDialog::setMaxDate(const QDate& maxDate)
{
  fromTime->setMaximumDate(maxDate);
}
void ListDialog::setMinDate(const QDate& minDate)
{
  toTime->setMinimumDate(minDate);
}

void ListDialog::showAll(){
  this->show();
}

void ListDialog::hideAll(){
  this->hide();
}

QString ListDialog::getStart() {
  //    return fromTime->isoTime().cStr();
  return fromTime->text() + QString(":00");
}

QString ListDialog::getWeatherElement() {
  return weatherElement;
}

void ListDialog::chooseParameters(const QString& str) {
  weatherElement = str;
 }

QString ListDialog::getEnd() {
  //  return toTime->isoTime().cStr();
  return toTime->text() + QString(":00");
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
    prcType->setChecked(false);
    aprType->setChecked(false);
    winType->setChecked(false);
    visType->setChecked(false);
    marType->setChecked(false);
  }
}

void ListDialog::prcCheck() {
  if (prcType->isChecked() ) {
    twiType->setChecked(false);
    aprType->setChecked(false);
    winType->setChecked(false);
    visType->setChecked(false);
    marType->setChecked(false);
  }
}

void ListDialog::aprCheck() {
  if (aprType->isChecked() ) {
    prcType->setChecked(false);
    twiType->setChecked(false);
    visType->setChecked(false);
    marType->setChecked(false);
  }
}

void ListDialog::winCheck() {
  if (winType->isChecked() ) {
    prcType->setChecked(false);
    twiType->setChecked(false);
    visType->setChecked(false);
    marType->setChecked(false);
  }
}

void ListDialog::visCheck() {
  if (visType->isChecked() ) {
    prcType->setChecked(false);
    aprType->setChecked(false);
    winType->setChecked(false);
    twiType->setChecked(false);
    marType->setChecked(false);
  }
}

void ListDialog::marCheck() {
  if (marType->isChecked() ) {
    prcType->setChecked(false);
    aprType->setChecked(false);
    winType->setChecked(false);
    twiType->setChecked(false);
    visType->setChecked(false);
  }
}

void ListDialog::uncheckTypes()
{
    mi_foreach(ItemCheckBox* cb, mStationTypes)
        cb->setChecked(false);
    allType->setChecked(false);
}

void ListDialog::checkTypes(const char* these[])
{
    mi_foreach(ItemCheckBox* cb, mStationTypes) {
        const QString item = cb->getItem();
        for(int i=0; these[i]; ++i) {
            if( item == these[i] ) {
                cb->setChecked(true);
                break;
            }
        }
    }
}

void ListDialog::otwiCheck()
{
    uncheckTypes();
    if( twiType->isChecked() ) {
        const char* doCheck[] = { "AA", "AF", "AL", "AV", "AO", "AE", "MV", "MP",
                                  "MM", "MS", "NS", "FM", "PT", "VS", "VK", "VM", 0 };
        checkTypes(doCheck);
    }
}

void ListDialog::oprcCheck()
{
    uncheckTypes();
    if( prcType->isChecked() ) {
        const char* doCheck[] = { "AA", "AL", "AO", "NS", "ND", "NO", "P", "VS", "VK", "VM", 0 };
        checkTypes(doCheck);
    }
}

void ListDialog::oaprCheck()
{
    uncheckTypes();
    if( aprType->isChecked() ) {
        const char* doCheck[] = { "AA", "AF", "AE", "MV", "MP", "MM", "MS", "VS", 0 };
        checkTypes(doCheck);
    }
    if( winType->isChecked() ) {
        const char* doCheck[] = { "AA", "AF", "AL", "AV", "AO", "AE", "MV", "MP", "MM", "MS", "FM", "VS", "VK", 0 };
        checkTypes(doCheck);
    }
}

void ListDialog::owinCheck()
{
    oaprCheck(); // TODO this does not seem right
}

void ListDialog::ovisCheck()
{
    uncheckTypes();
    if( visType->isChecked() ) {
        const char* doCheck[] = { "MV", "MP", "MM", "MS", "FM", "NS", "ND", "NO", "VS", "VK", "VM", 0 };
        checkTypes(doCheck);
    }
}

void ListDialog::omarCheck() {
    uncheckTypes();
    if ( marType->isChecked() ) {
        const char* doCheck[] = { "MV", "MP", "MM", "MS", 0 };
        checkTypes(doCheck);
    }
}

void ListDialog::ausCheck() {
  if (ausReg->isChecked() ) {
    vesReg->setChecked(false);
    troReg->setChecked(false);
    norReg->setChecked(false);
    webReg->setChecked(false);
    priReg->setChecked(false);
  }
}

void ListDialog::oausCheck() {
  oslCoun->setChecked(false);
  akeCoun->setChecked(false);
  ostCoun->setChecked(false);
  hedCoun->setChecked(false);
  oppCoun->setChecked(false);
  busCoun->setChecked(false);
  vefCoun->setChecked(false);
  telCoun->setChecked(false);
  ausCoun->setChecked(false);
  veaCoun->setChecked(false);
  rogCoun->setChecked(false);
  horCoun->setChecked(false);
  sogCoun->setChecked(false);
  morCoun->setChecked(false);
  sorCoun->setChecked(false);
  ntrCoun->setChecked(false);
  norCoun->setChecked(false);
  troCoun->setChecked(false);
  finCoun->setChecked(false);
  svaCoun->setChecked(false);
  allCoun->setChecked(false);
  if ( ausReg->isChecked() ) {
    oslCoun->setChecked(true);
    akeCoun->setChecked(true);
    ostCoun->setChecked(true);
    hedCoun->setChecked(true);
    oppCoun->setChecked(true);
    busCoun->setChecked(true);
    vefCoun->setChecked(true);
    telCoun->setChecked(true);
    ausCoun->setChecked(true);
  }
}

void ListDialog::vesCheck() {
  if (vesReg->isChecked() ) {
    ausReg->setChecked(false);
    troReg->setChecked(false);
    norReg->setChecked(false);
    webReg->setChecked(false);
    priReg->setChecked(false);
  }
}

void ListDialog::ovesCheck() {
  oslCoun->setChecked(false);
  akeCoun->setChecked(false);
  ostCoun->setChecked(false);
  hedCoun->setChecked(false);
  oppCoun->setChecked(false);
  busCoun->setChecked(false);
  vefCoun->setChecked(false);
  telCoun->setChecked(false);
  ausCoun->setChecked(false);
  veaCoun->setChecked(false);
  rogCoun->setChecked(false);
  horCoun->setChecked(false);
  sogCoun->setChecked(false);
  morCoun->setChecked(false);
  sorCoun->setChecked(false);
  ntrCoun->setChecked(false);
  norCoun->setChecked(false);
  troCoun->setChecked(false);
  finCoun->setChecked(false);
  svaCoun->setChecked(false);
  allCoun->setChecked(false);
  if ( vesReg->isChecked() ) {
    veaCoun->setChecked(true);
    rogCoun->setChecked(true);
    horCoun->setChecked(true);
    sogCoun->setChecked(true);
    morCoun->setChecked(true);
  }
}

void ListDialog::troCheck() {
  if (troReg->isChecked() ) {
    ausReg->setChecked(false);
    vesReg->setChecked(false);
    norReg->setChecked(false);
    webReg->setChecked(false);
    priReg->setChecked(false);
  }
}

void ListDialog::otroCheck() {
  oslCoun->setChecked(false);
  akeCoun->setChecked(false);
  ostCoun->setChecked(false);
  hedCoun->setChecked(false);
  oppCoun->setChecked(false);
  busCoun->setChecked(false);
  vefCoun->setChecked(false);
  telCoun->setChecked(false);
  ausCoun->setChecked(false);
  veaCoun->setChecked(false);
  rogCoun->setChecked(false);
  horCoun->setChecked(false);
  sogCoun->setChecked(false);
  morCoun->setChecked(false);
  sorCoun->setChecked(false);
  ntrCoun->setChecked(false);
  norCoun->setChecked(false);
  troCoun->setChecked(false);
  finCoun->setChecked(false);
  svaCoun->setChecked(false);
  allCoun->setChecked(false);
  if ( troReg->isChecked() ) {
    sorCoun->setChecked(true);
    ntrCoun->setChecked(true);
  }
}

void ListDialog::norCheck() {
  if (norReg->isChecked() ) {
    vesReg->setChecked(false);
    troReg->setChecked(false);
    ausReg->setChecked(false);
    webReg->setChecked(false);
    priReg->setChecked(false);
  }
}

void ListDialog::onorCheck() {
  oslCoun->setChecked(false);
  akeCoun->setChecked(false);
  ostCoun->setChecked(false);
  hedCoun->setChecked(false);
  oppCoun->setChecked(false);
  busCoun->setChecked(false);
  vefCoun->setChecked(false);
  telCoun->setChecked(false);
  ausCoun->setChecked(false);
  veaCoun->setChecked(false);
  rogCoun->setChecked(false);
  horCoun->setChecked(false);
  sogCoun->setChecked(false);
  morCoun->setChecked(false);
  sorCoun->setChecked(false);
  ntrCoun->setChecked(false);
  norCoun->setChecked(false);
  troCoun->setChecked(false);
  finCoun->setChecked(false);
  svaCoun->setChecked(false);
  allCoun->setChecked(false);
  if ( norReg->isChecked() ) {
    norCoun->setChecked(true);
    troCoun->setChecked(true);
    finCoun->setChecked(true);
    svaCoun->setChecked(true);
  }
}

void ListDialog::webCheck() {
  if (webReg->isChecked() ) {
    allType->setChecked(true);
  }
}

void ListDialog::owebCheck() {
  oslCoun->setChecked(false);
  akeCoun->setChecked(false);
  ostCoun->setChecked(false);
  hedCoun->setChecked(false);
  oppCoun->setChecked(false);
  busCoun->setChecked(false);
  vefCoun->setChecked(false);
  telCoun->setChecked(false);
  ausCoun->setChecked(false);
  veaCoun->setChecked(false);
  rogCoun->setChecked(false);
  horCoun->setChecked(false);
  sogCoun->setChecked(false);
  morCoun->setChecked(false);
  sorCoun->setChecked(false);
  ntrCoun->setChecked(false);
  norCoun->setChecked(false);
  troCoun->setChecked(false);
  finCoun->setChecked(false);
  svaCoun->setChecked(false);
  allCoun->setChecked(false);
}
void ListDialog::priCheck() {
  if (priReg->isChecked() ) {
    allType->setChecked(true);
  }
}

void ListDialog::opriCheck() {
  oslCoun->setChecked(false);
  akeCoun->setChecked(false);
  ostCoun->setChecked(false);
  hedCoun->setChecked(false);
  oppCoun->setChecked(false);
  busCoun->setChecked(false);
  vefCoun->setChecked(false);
  telCoun->setChecked(false);
  ausCoun->setChecked(false);
  veaCoun->setChecked(false);
  rogCoun->setChecked(false);
  horCoun->setChecked(false);
  sogCoun->setChecked(false);
  morCoun->setChecked(false);
  sorCoun->setChecked(false);
  ntrCoun->setChecked(false);
  norCoun->setChecked(false);
  troCoun->setChecked(false);
  finCoun->setChecked(false);
  svaCoun->setChecked(false);
  allCoun->setChecked(false);
}
void ListDialog::allCounCheck() {
  if ( allCoun->isChecked() ) {
    oslCoun->setChecked(false);
    akeCoun->setChecked(false);
    ostCoun->setChecked(false);
    hedCoun->setChecked(false);
    oppCoun->setChecked(false);
    busCoun->setChecked(false);
    vefCoun->setChecked(false);
    telCoun->setChecked(false);
    ausCoun->setChecked(false);
    veaCoun->setChecked(false);
    rogCoun->setChecked(false);
    horCoun->setChecked(false);
    sogCoun->setChecked(false);
    morCoun->setChecked(false);
    sorCoun->setChecked(false);
    ntrCoun->setChecked(false);
    norCoun->setChecked(false);
    troCoun->setChecked(false);
    finCoun->setChecked(false);
    svaCoun->setChecked(false);
    vesReg->setChecked(false);
    troReg->setChecked(false);
    ausReg->setChecked(false);
    norReg->setChecked(false);
    priReg->setChecked(false);
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
      allCoun->setChecked(false);
  }
}

QStringList ListDialog::getSelectedStationTypes()
{
    QStringList t;
    if( allType->isChecked() )
        t << "ALL";
    mi_foreach(ItemCheckBox* cb, mStationTypes) {
        if( cb->isChecked() )
            t << cb->getItem();
    }
    return t;
}

QStringList ListDialog::getSelectedCounties()
{
    QStringList t;
    if( allCoun->isChecked() )
        t << "ALL";
    if( oslCoun->isChecked() ) t <<  "OSLO";
    if( akeCoun->isChecked() ) t <<  "AKERSHUS";
    if( ostCoun->isChecked() ) t <<  "ØSTFOLD";
    if( hedCoun->isChecked() ) t <<  "HEDMARK";
    if( oppCoun->isChecked() ) t <<  "OPPLAND";
    if( busCoun->isChecked() ) t <<  "BUSKERUD";
    if( vefCoun->isChecked() ) t <<  "VESTFOLD";
    if( telCoun->isChecked() ) t <<  "TELEMARK";
    if( ausCoun->isChecked() ) t <<  "AUST-AGDER";
    if( veaCoun->isChecked() ) t <<  "VEST-AGDER";
    if( rogCoun->isChecked() ) t <<  "ROGALAND";
    if( horCoun->isChecked() ) t <<  "HORDALAND";
    if( sogCoun->isChecked() ) t <<  "SOGN OG FJORDANE";
    if( morCoun->isChecked() ) t <<  "MØRE OG ROMSDAL";
    if( sorCoun->isChecked() ) t <<  "SØR-TRØNDELAG";
    if( ntrCoun->isChecked() ) t <<  "NORD-TRØNDELAG";
    if( norCoun->isChecked() ) t <<  "NORDLAND";
    if( troCoun->isChecked() ) t <<  "TROMS";
    if( finCoun->isChecked() ) t <<  "FINNMARK";
    if( svaCoun->isChecked() ) t <<  "ISHAVET";
    return t;
}

void ListDialog::setSelectedStationTypes(const QStringList& stationTypes)
{
    allType->setChecked(stationTypes.contains("ALL"));
    mi_foreach(ItemCheckBox* cb, mStationTypes)
        cb->setChecked(stationTypes.contains(cb->getItem()));
}

StationTable::StationTable(const listStat_l& listStat,
                           const QStringList& stationTypes,
                           const QStringList& counties,
			   bool web,
			   bool pri,
			   ObsTypeList* otpList,
			   QWidget* parent)
    : Q3Table(listStat.size(), 7, parent)
{
  setCaption(tr("Stasjoner"));
  setSorting( true );
  setGeometry(10,100,800,600);

  horizontalHeader()->setLabel( 0, tr( "Stnr" ) );
  horizontalHeader()->setLabel( 1, tr( "Navn" ) );
  horizontalHeader()->setLabel( 2, tr( "HOH" ) );
  horizontalHeader()->setLabel( 3, tr( "Type" ) );
  horizontalHeader()->setLabel( 4, tr( "Fylke" ) );
  horizontalHeader()->setLabel( 5, tr( "Kommune" ) );
  horizontalHeader()->setLabel( 6, tr( "Pri" ) );
  int stInd = 0;
  mi_foreach(const listStat_t& s, listStat) {
    bool webStat = (s.wmonr != "    ");
    bool priStat = (s.pri.substr(0, 3) == "PRI");
    QString prty;
    if( s.pri.size() >= 4 )
        prty = QString::fromStdString(s.pri.substr(3,1));

    bool foundStat = false;
    ObsTypeList::iterator oit = otpList->begin();
    for ( ; oit != otpList->end(); oit++) {
      TypeList::iterator tit = oit->begin();
      if( s.stationid == (*tit) ) {
	foundStat = true;
	break;
      }
    }
    if ( !foundStat ) {
      continue;
    }
    if ( ! (counties.contains("ALL") ||
	    counties.contains(QString::fromStdString(s.fylke)) ||
       	    (webStat && web) || (priStat && pri) ))
      continue;
    QString strEnv;
    if( stationTypes.contains("ALL") ) {
        strEnv = getEnvironment(s.environment, oit);
    } else {
        if ( (stationTypes.contains("AA") && ((s.environment == 8 && (findInTypes(oit, 3)  || findInTypes(oit, 311))) || findInTypes(oit, 330) || findInTypes(oit, 342))) ) strEnv += "AA";
        if ( (stationTypes.contains("AF") && s.environment == 1 && findInTypes(oit, 311)) )  strEnv += "AF";
        if ( (stationTypes.contains("AL") && s.environment == 2 && findInTypes(oit, 3)) ) strEnv += "AL";
        if ( (stationTypes.contains("AV") && s.environment == 12 && findInTypes(oit, 3)) )  strEnv += "AV";
        if ( (stationTypes.contains("AO") && findInTypes(oit, 410)) )  strEnv += "AO";
        if ( (stationTypes.contains("MV") && s.environment == 7 && findInTypes(oit, 11)) ) strEnv += "MV";
        if ( (stationTypes.contains("MP") && s.environment == 5 && findInTypes(oit, 11)) ) strEnv += "MP";
        if ( (stationTypes.contains("MM") && s.environment == 4 && findInTypes(oit, 11)) ) strEnv += "MM";
        if ( (stationTypes.contains("MS") && s.environment == 6 && findInTypes(oit, 11)) ) strEnv += "MS";
        if ( (stationTypes.contains("P")  && (findInTypes(oit, 4) || findInTypes(oit, 404))) ) strEnv += "P";
        if ( (stationTypes.contains("PT") && (findInTypes(oit, 4) || findInTypes(oit, 404))) ) strEnv += "PT";
        if ( (stationTypes.contains("NS") && findInTypes(oit, 302)) )  strEnv += "NS";
        if ( (stationTypes.contains("ND") && s.environment == 9 && findInTypes(oit, 402)) ) strEnv += "ND";
        if ( (stationTypes.contains("NO") && s.environment == 10 && findInTypes(oit, 402)) ) strEnv += "NO";
        if ( (stationTypes.contains("VS") && (findInTypes(oit, 1) || findInTypes(oit, 6) || findInTypes(oit, 312))) ) strEnv += "VS";
        if ( (stationTypes.contains("VK") && s.environment == 3 && findInTypes(oit, 412)) ) strEnv += "VK";
        if ( (stationTypes.contains("VM") && (findInTypes(oit, 306) || findInTypes(oit, 308))) ) strEnv += "VM";
        if ( (stationTypes.contains("FM") && (findInTypes(oit, 2))) ) strEnv += "FM";
    }
    if ( not strEnv.isEmpty() ) {
        StTableItem* stNum = new StTableItem(this, Q3TableItem::Never, QString::number(s.stationid));
        setItem(stInd, 0, stNum);
        StTableItem* stName = new StTableItem(this, Q3TableItem::Never, QString::fromStdString(s.name));
        setItem(stInd, 1, stName);
        StTableItem* stHoh = new StTableItem(this, Q3TableItem::Never, QString::number(s.altitude, 'f', 0));
        setItem(stInd, 2, stHoh);
        StTableItem* stType = new StTableItem(this, Q3TableItem::Never, strEnv);
        setItem(stInd, 3, stType);
        StTableItem* stFylke = new StTableItem(this, Q3TableItem::Never, QString::fromStdString(s.fylke));
        setItem(stInd, 4, stFylke);
        StTableItem* stKommune = new StTableItem(this, Q3TableItem::Never, QString::fromStdString(s.kommune));
        setItem(stInd, 5, stKommune);
        StTableItem* stPrior = new StTableItem(this, Q3TableItem::Never, prty);
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
    sortColumn(6, true, true);
  else
    hideColumn(6);
}

bool StationTable::findInTypes(ObsTypeList::iterator tList, int type)
{
    if( tList->empty() )
        return false;
    // '++' in next is necessary as the first entry in tList is not a
    // typeId but a station id number
    return std::find(++tList->begin(), tList->end(), type) != tList->end();
}

QString StationTable::getEnvironment(const int envID, ObsTypeList::iterator oit) {
  QString env;
  if ( envID == 1 && findInTypes(oit, 311) )
    env = "AF";
  else if ( envID == 2 && findInTypes(oit, 3) )
    env = "AL";
  else if ( envID == 4 && findInTypes(oit, 11) )
    env = "MM";
  else if ( envID == 5 && findInTypes(oit, 11) )
    env = "MP";
  else if ( envID == 6 && findInTypes(oit, 11) )
    env = "MS";
  else if ( envID == 7 && findInTypes(oit, 11) )
    env = "MV";
  else if ( (envID == 8 && (findInTypes(oit, 3)  || findInTypes(oit, 311))) || findInTypes(oit, 330) || findInTypes(oit, 342) )
    env = "AA";
  else if ( envID == 9 && findInTypes(oit, 402) )
    env = "ND";
  else if ( envID == 10 && findInTypes(oit, 402) )
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
  else if ( envID == 11 )
    env = "TURISTFORENING";
  else if ( envID == 12 && findInTypes(oit, 3) )
    env = "AV";
  else if ( findInTypes(oit, 412) )
    env = "VK";
  //  else if ( findInTypes(oit, 503) )
  else if ( findInTypes(oit, 502) || findInTypes(oit, 503) || findInTypes(oit, 504) || findInTypes(oit, 505) || findInTypes(oit, 506) || findInTypes(oit, 514) )
    env = "X";
  return env;
}


void StationTable::sortColumn( int col, bool ascending, bool /*wholeRows*/ ) {
    Q3Table::sortColumn( col, ascending, true );
}

StationSelection::StationSelection(const listStat_l& listStat,
                                   const QStringList& stationTypes,
                                   const QStringList& counties,
				   bool web,
				   bool pri,
				   ObsTypeList* otpList,
                                   QWidget* parent)
    : QDialog(parent)
{
    // setGeometry(0,0,870,700);
    resize(870,700);

  selectionOK = new QPushButton("Lukk", this);
  selectionOK->setGeometry(50,10,130,30);
  selectionOK->setDefault(true);
  connect(selectionOK, SIGNAL(clicked()),SLOT(listSelectedStations()));

  selectAllStations = new QPushButton("Velg &alle stasjoner", this);
  selectAllStations->setGeometry(200,10,130,30);
  connect(selectAllStations, SIGNAL(clicked()),SLOT(showAllStations()));

  stationTable = new StationTable(listStat,
                                  stationTypes,
                                  counties,
				  web,
				  pri,
				  otpList,
				  this);
  connect( stationTable,SIGNAL(currentChanged(int, int)),
	   SLOT(tableCellClicked(int, int)));
}

void StationSelection::tableCellClicked() {
}
void StationSelection::tableCellClicked(int row,
					int /*col*/,
					int /*button*/,
					const QPoint& /*mousePos*/) {
  stationTable->selectRow(row);
  showSelectedStation(row, 0);
}

void StationSelection::tableCellClicked(int row, int /*col*/) {
  stationTable->selectRow(row);
  showSelectedStation(row, 0);
}

void StationSelection::showSelectedStation(int row, int /*col*/) {
  Q3TableItem* tStationNumber = stationTable->item( row, 0);
  Q3TableItem* tStationName = stationTable->item( row, 1);
  QString station = tStationNumber->text() + "  " + tStationName->text();
  int rem = stlist.remove(station); // FIXME kind of weird procedure
  if ( rem == 0 ) {
      stlist.append(station);
    emit stationAppended(station);
  } else {
    emit stationRemoved(station);
  }
}

void StationSelection::showAllStations() {
  for (  int row = 0; row < stationTable->numRows(); row++ ) {
    Q3TableItem* tStationNumber = stationTable->item( row, 0);
    Q3TableItem* tStationName = stationTable->item( row, 1);
    QString station = tStationNumber->text() + "  " + tStationName->text();
    int rem = stlist.remove(station); // FIXME kind of weird procedure
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
