/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2013 met.no

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

#include "BusyIndicator.h"
#include "HideApplyBox.hh"
#include "hqcmain.h"
#include "KvMetaDataBuffer.hh"
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
} // anonymous namespace

void ItemCheckBox::clicked()
{
    /*emit*/ clicked(mItem);
}

ListDialog::ListDialog(HqcMainWindow* parent)
  : QDialog(parent)
  , statSelect(0)
{
    setupUi(this);

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

  for(int i=0; i<NSTATIONTYPES; ++i) {
      const stationtype_t& s = stationTypes[i];
      ItemCheckBox* cb = new ItemCheckBox(s.name, s.name, stTyp);
      statSelLayout->addWidget(cb, s.gridX, s.gridY);
      mStationTypes.push_back(cb);
  }
  allType = new QCheckBox( tr("Alle"), stTyp);
  statSelLayout->addWidget(allType, 5, 3);

  // insert checkbuttons for station location selection
  int x=0, y=0;
  ItemCheckBox** countiesCB[NCOUNTIES] = {
      &oslCoun, &akeCoun, &ostCoun, &hedCoun, &oppCoun, &busCoun, &vefCoun,
      &telCoun, &ausCoun, &veaCoun, &rogCoun, &horCoun, &sogCoun, &morCoun,
      &sorCoun, &ntrCoun, &norCoun, &troCoun, &finCoun, &svaCoun
  };
  for(int i=0; i<NCOUNTIES; ++i) {
      *countiesCB[i] = new ItemCheckBox(counties[i], countiesU[i], stCounty);
      statCountyLayout->addWidget(*countiesCB[i], x, y);
      y += 1; if( y >= 3 ) { y = 0; x += 1; }
  }
  allCoun = new ItemCheckBox(tr("Alle"), "ALL", stCounty);
  statCountyLayout->addWidget(allCoun, x, y);

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

  connect(stationSelect, SIGNAL(clicked()), this, SLOT(showStationSelectionDialog()));

  //Time selection
  QDateTime t = timeutil::nowWithMinutes0Seconds0();
  QDateTime f = t.addSecs(-2*24*3600 + 3600*(17-t.time().hour()) + 60*45);
  fromTime->setDateTime(f);
  toTime->setDateTime(t);

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

  hab->setCanApply(false);
  connect(hab, SIGNAL(hide()), this, SIGNAL(ListHide()));
  connect(hab, SIGNAL(apply()), this, SIGNAL(ListApply()));
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

QDateTime ListDialog::getStart()
{
    return fromTime->dateTime();
}

QDateTime ListDialog::getEnd()
{
    return toTime->dateTime();
}

void ListDialog::setEnd(const QDateTime& e)
{
    toTime->setDateTime(e);
}

void ListDialog::appendStatInListbox(QString station)
{
    stationNames->insertItem(station);
    hab->setCanApply(true);
}

void ListDialog::removeStatFromListbox(QString station)
{
    int rind = -1;
    for (  int ind = 0; ind < stationNames->numRows(); ind++ ) {
        if ( stationNames->text(ind) == station ) {
            rind = ind;
        }
    }
    if ( rind >= 0 )
        stationNames->removeItem(rind);
    if( stationNames->count() == 0 )
        hab->setCanApply(false);
}

void ListDialog::removeAllStatFromListbox()
{
    int nuRo = stationNames->count();
    for (  int ind = 0; ind < nuRo; ind++ ) {
        stationNames->removeItem(0);
    }
    hab->setCanApply(false);
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

void ListDialog::setSelectedStationTypes(const QStringList& stationTypes)
{
    allType->setChecked(stationTypes.contains("ALL"));
    mi_foreach(ItemCheckBox* cb, mStationTypes)
        cb->setChecked(stationTypes.contains(cb->getItem()));
}

QStringList ListDialog::getSelectedCounties()
{
    QStringList t;

    const int NBOXES = 7;
    QCheckBox* boxes[NBOXES] = { allCoun, ausReg,     vesReg,     troReg,          norReg,      webReg,     priReg };
    QString    keys[NBOXES]  = { "ALL",   "REG_EAST", "REG_WEST", "REG_TRONDELAG", "REG_NORTH", "ST_SYNOP", "ST_PRIO" };
    for(int i=0; i<NBOXES; ++i) {
        if (boxes[i]->isChecked())
            t << keys[i];
    }

    ItemCheckBox** countiesCB[NCOUNTIES] = {
        &oslCoun, &akeCoun, &ostCoun, &hedCoun, &oppCoun, &busCoun, &vefCoun,
        &telCoun, &ausCoun, &veaCoun, &rogCoun, &horCoun, &sogCoun, &morCoun,
        &sorCoun, &ntrCoun, &norCoun, &troCoun, &finCoun, &svaCoun
    };
    for(int i=0; i<NCOUNTIES; ++i) {
        ItemCheckBox* cb = *countiesCB[i];
        if( cb->isChecked() )
            t << cb->getItem();
    }
    return t;
}

void ListDialog::setSelectedCounties(const QStringList& c)
{
    const int NBOXES = 7;
    QCheckBox* boxes[NBOXES] = { allCoun, ausReg,     vesReg,     troReg,          norReg,      webReg,     priReg };
    QString    keys[NBOXES]  = { "ALL",   "REG_EAST", "REG_WEST", "REG_TRONDELAG", "REG_NORTH", "ST_SYNOP", "ST_PRIO" };
    for(int i=0; i<NBOXES; ++i)
        boxes[i]->setChecked(c.contains(keys[i]));

    ItemCheckBox** countiesCB[NCOUNTIES] = {
        &oslCoun, &akeCoun, &ostCoun, &hedCoun, &oppCoun, &busCoun, &vefCoun,
        &telCoun, &ausCoun, &veaCoun, &rogCoun, &horCoun, &sogCoun, &morCoun,
        &sorCoun, &ntrCoun, &norCoun, &troCoun, &finCoun, &svaCoun
    };
    for(int i=0; i<NCOUNTIES; ++i) {
        ItemCheckBox* cb = *countiesCB[i];
        cb->setChecked(c.contains(cb->getItem()));
    }
}

std::vector<int> ListDialog::getSelectedStations()
{
    if( statSelect )
        return statSelect->getSelectedStations();
    else
        return std::vector<int>();
}

void ListDialog::showStationSelectionDialog()
{
    const std::list<listStat_t>& listStat = static_cast<HqcMainWindow*>(parent())->getStationDetails();

    removeAllStatFromListbox();
    if( statSelect )
        delete statSelect;

    statSelect = new StationSelection(listStat,
                                      getSelectedStationTypes(),
                                      getSelectedCounties(),
                                      showSynop(),
                                      showPrioritized(),
                                      this);
    connect(statSelect, SIGNAL(stationAppended(QString)), this, SLOT(appendStatInListbox(QString)));
    connect(statSelect, SIGNAL(stationRemoved(QString)),  this, SLOT(removeStatFromListbox(QString)));
    statSelect->show();
}

// ########################################################################
// ########################################################################
// ########################################################################

StationTable::StationTable(QWidget* parent)
    : Q3Table(0, 7, parent)
{
  setSorting( true );

  horizontalHeader()->setLabel( 0, tr( "Stnr" ) );
  horizontalHeader()->setLabel( 1, tr( "Navn" ) );
  horizontalHeader()->setLabel( 2, tr( "HOH" ) );
  horizontalHeader()->setLabel( 3, tr( "Type" ) );
  horizontalHeader()->setLabel( 4, tr( "Fylke" ) );
  horizontalHeader()->setLabel( 5, tr( "Kommune" ) );
  horizontalHeader()->setLabel( 6, tr( "Pri" ) );
}

void StationTable::setData(const listStat_l& listStat,
                           const QStringList& stationTypes,
                           const QStringList& counties,
			   bool web,
			   bool pri)
{
  BusyIndicator busy;
  setNumRows(listStat.size());

  int stInd = 0;
  mi_foreach(const listStat_t& s, listStat) {
    bool webStat = (s.wmonr != "    ");
    bool priStat = (s.pri.substr(0, 3) == "PRI");
    QString prty;
    if( s.pri.size() >= 4 )
        prty = QString::fromStdString(s.pri.substr(3,1));

    if (not (counties.contains("ALL")
             or counties.contains(QString::fromStdString(s.fylke))
             or (webStat and web) or (priStat or pri)))
        continue;

    const std::list<kvalobs::kvObsPgm>& obsPgmList = KvMetaDataBuffer::instance()->findObsPgm(s.stationid);
    std::set<int> typeIDs;
    mi_foreach(const kvalobs::kvObsPgm& op, obsPgmList)
        typeIDs.insert(op.typeID());

    QString strEnv;
    if( stationTypes.contains("ALL") ) {
        strEnv = getEnvironment(s.environment, typeIDs);
    } else {
        if ( (stationTypes.contains("AA") && ((s.environment == 8 && (typeIDs.count(3)  || typeIDs.count(311))) || typeIDs.count(330) || typeIDs.count(342))) ) strEnv += "AA";
        if ( (stationTypes.contains("AF") && s.environment == 1 && typeIDs.count(311)) )  strEnv += "AF";
        if ( (stationTypes.contains("AL") && s.environment == 2 && typeIDs.count(3)) ) strEnv += "AL";
        if ( (stationTypes.contains("AV") && s.environment == 12 && typeIDs.count(3)) )  strEnv += "AV";
        if ( (stationTypes.contains("AO") && typeIDs.count(410)) )  strEnv += "AO";
        if ( (stationTypes.contains("MV") && s.environment == 7 && typeIDs.count(11)) ) strEnv += "MV";
        if ( (stationTypes.contains("MP") && s.environment == 5 && typeIDs.count(11)) ) strEnv += "MP";
        if ( (stationTypes.contains("MM") && s.environment == 4 && typeIDs.count(11)) ) strEnv += "MM";
        if ( (stationTypes.contains("MS") && s.environment == 6 && typeIDs.count(11)) ) strEnv += "MS";
        if ( (stationTypes.contains("P")  && (typeIDs.count(4) || typeIDs.count(404))) ) strEnv += "P";
        if ( (stationTypes.contains("PT") && (typeIDs.count(4) || typeIDs.count(404))) ) strEnv += "PT";
        if ( (stationTypes.contains("NS") && typeIDs.count(302)) )  strEnv += "NS";
        if ( (stationTypes.contains("ND") && s.environment == 9 && typeIDs.count(402)) ) strEnv += "ND";
        if ( (stationTypes.contains("NO") && s.environment == 10 && typeIDs.count(402)) ) strEnv += "NO";
        if ( (stationTypes.contains("VS") && (typeIDs.count(1) || typeIDs.count(6) || typeIDs.count(312))) ) strEnv += "VS";
        if ( (stationTypes.contains("VK") && s.environment == 3 && typeIDs.count(412)) ) strEnv += "VK";
        if ( (stationTypes.contains("VM") && (typeIDs.count(306) || typeIDs.count(308))) ) strEnv += "VM";
        if ( (stationTypes.contains("FM") && (typeIDs.count(2))) ) strEnv += "FM";
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

QString StationTable::getEnvironment(const int envID, const std::set<int>& typeIDs) {
  QString env;
  if ( envID == 1 && typeIDs.count(311) )
    env = "AF";
  else if ( envID == 2 && typeIDs.count(3) )
    env = "AL";
  else if ( envID == 4 && typeIDs.count(11) )
    env = "MM";
  else if ( envID == 5 && typeIDs.count(11) )
    env = "MP";
  else if ( envID == 6 && typeIDs.count(11) )
    env = "MS";
  else if ( envID == 7 && typeIDs.count(11) )
    env = "MV";
  else if ( (envID == 8 && (typeIDs.count(3)  || typeIDs.count(311))) || typeIDs.count(330) || typeIDs.count(342) )
    env = "AA";
  else if ( envID == 9 && typeIDs.count(402) )
    env = "ND";
  else if ( envID == 10 && typeIDs.count(402) )
    env = "NO";
  else if ( typeIDs.count(302) )
    env = "NS";
  else if ( typeIDs.count(410) )
    env = "AO";
  else if ( typeIDs.count(4) || typeIDs.count(404) )
    env = "P,PT";
  else if ( typeIDs.count(2) )
    env = "FM";
  else if ( typeIDs.count(1) || typeIDs.count(6) || typeIDs.count(312) )
    env = "VS";
  else if ( typeIDs.count(306) || typeIDs.count(308) )
    env = "VM";
  else if ( envID == 11 )
    env = "TURISTFORENING";
  else if ( envID == 12 && typeIDs.count(3) )
    env = "AV";
  else if ( typeIDs.count(412) )
    env = "VK";
  //  else if ( typeIDs.count(503) )
  else if ( typeIDs.count(502) || typeIDs.count(503) || typeIDs.count(504) || typeIDs.count(505) || typeIDs.count(506) || typeIDs.count(514) )
    env = "X";
  return env;
}


void StationTable::sortColumn( int col, bool ascending, bool /*wholeRows*/ ) {
    Q3Table::sortColumn( col, ascending, true );
}

// ########################################################################
// ########################################################################
// ########################################################################

StationSelection::StationSelection(const listStat_l& listStat,
                                   const QStringList& stationTypes,
                                   const QStringList& counties,
				   bool web,
				   bool pri,
                                   QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(selectionOK, SIGNAL(clicked()), this, SLOT(hide()));
    connect(selectAllStations, SIGNAL(clicked()),SLOT(doSelectAllStations()));

    stationTable->setData(listStat, stationTypes, counties, web, pri);
    connect(stationTable,SIGNAL(currentChanged(int, int)),
            SLOT(tableCellClicked(int, int)));
}

void StationSelection::tableCellClicked() {
}
void StationSelection::tableCellClicked(int row,
					int /*col*/,
					int /*button*/,
					const QPoint& /*mousePos*/) {
  stationTable->selectRow(row);
  selectOrDeselectStation(row);
}

void StationSelection::tableCellClicked(int row, int /*col*/) {
  stationTable->selectRow(row);
  selectOrDeselectStation(row);
}

void StationSelection::selectOrDeselectStation(int row)
{
    Q3TableItem* tStationNumber = stationTable->item( row, 0);
    Q3TableItem* tStationName = stationTable->item( row, 1);
    QString station = tStationNumber->text() + "  " + tStationName->text();
    
    const int stationID = tStationNumber->text().toInt();
    std::set<int>::iterator it = mSelectedStations.find(stationID);
    
    if( it != mSelectedStations.end() ) {
        mSelectedStations.erase(it);
        /*emit*/ stationRemoved(station);
    } else {
        mSelectedStations.insert(stationID);
        /*emit*/ stationAppended(station);
    }
}

void StationSelection::doSelectAllStations()
{
    for(int row = 0; row < stationTable->numRows(); row++) {
        Q3TableItem* tStationNumber = stationTable->item( row, 0);
        
        const int stationID = tStationNumber->text().toInt();
        if( mSelectedStations.find(stationID) == mSelectedStations.end() ) {
            mSelectedStations.insert(stationID);

            Q3TableItem* tStationName = stationTable->item( row, 1);
            QString station = tStationNumber->text() + "  " + tStationName->text();
            /*emit*/ stationAppended(station);
        }
    }
}

std::vector<int> StationSelection::getSelectedStations()
{
    return std::vector<int>(mSelectedStations.begin(), mSelectedStations.end());
}

// ########################################################################
// ########################################################################
// ########################################################################

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
