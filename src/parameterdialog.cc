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

#include <QtCore/QStringList>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include <QtCore/QDebug>

ParameterDialog::ParameterDialog(QWidget* parent): QDialog(parent) {
  setCaption(tr("Parametervalg"));
  setGeometry(500,10,300,580);

  QVBoxLayout * vl = new QVBoxLayout(this,10);

  QGroupBox *pVal = new QGroupBox(tr("Parametervalg"), this);

  plb = new QListWidget(this);
  plb->setSelectionMode( QAbstractItemView::MultiSelection );
  connect(plb,SIGNAL(itemSelectionChanged()),SLOT(selectionChanged()));

  allPar    = new QRadioButton( tr("Velg alle parametere"), pVal );
  markPar   = new QRadioButton( tr("Velg merkede parametere"), pVal );
  noMarkPar = new QRadioButton( tr("Velg bort merkede parametere"), pVal );

  QVBoxLayout * rbvl = new QVBoxLayout(this,10);
  rbvl->addWidget(allPar);
  rbvl->addWidget(markPar);
  rbvl->addWidget(noMarkPar);

  QFont smallFont("Arial", 9);

  sthide = new QPushButton(tr("Skjul"), this);
  sthide->setFont(smallFont);

  excu = new QPushButton(tr("Utfør"), this);
  excu->setFont(smallFont);

  hdnexcu = new QPushButton(tr("Utfør+Skjul"), this);
  hdnexcu->setFont(smallFont);
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
  selectionChanged();
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

void ParameterDialog::selectionChanged()
{
    const bool haveSelected = (plb->selectedItems().size() > 0);
    if( haveSelected && allPar->isChecked() )
        markPar->setChecked(true);
    else if( !haveSelected && !allPar->isChecked() )
        allPar->setChecked(true);
    allPar->setDisabled(haveSelected);
    markPar->setDisabled(!haveSelected);
    noMarkPar->setDisabled(!haveSelected);
}

bool ParameterDialog::isSelectedParameter(int paramIndex) const
{
    return plb->item(paramIndex)->isSelected();
}
