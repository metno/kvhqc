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
#include "HideApplyBox.hh"

ParameterDialog::ParameterDialog(QWidget* parent)
  : QDialog(parent)
{
    setupUi(this);

    connect(plb, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
    connect(hab, SIGNAL(hide()),  this, SIGNAL(paramHide()));
    connect(hab, SIGNAL(apply()), this, SIGNAL(paramApply()));

    selectionChanged();
}

void ParameterDialog::insertParametersInListBox(const std::vector<int> & porder, const QMap<int,QString> & parMap) {
  plb->clear();
  int jj = 0;
  for (std::vector<int>::const_iterator it = porder.begin(); it != porder.end(); ++it) {
      QString sp = parMap[*it];
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
