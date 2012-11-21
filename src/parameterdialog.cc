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

#include <QtCore/QStringList>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include <QtCore/QDebug>

ParameterDialog::ParameterDialog(QWidget* parent)
  : QDialog(parent)
{
    setCaption(tr("Parametervalg"));
    resize(300,580);


  QGroupBox *pVal = new QGroupBox(tr("Parametervalg"), this);

  plb = new QListWidget(this);
  plb->setSelectionMode( QAbstractItemView::MultiSelection );
  connect(plb,SIGNAL(itemSelectionChanged()),SLOT(selectionChanged()));

  allPar    = new QRadioButton( tr("Velg alle parametere"), pVal );
  markPar   = new QRadioButton( tr("Velg merkede parametere"), pVal );
  noMarkPar = new QRadioButton( tr("Velg bort merkede parametere"), pVal );

  QVBoxLayout * rbvl = new QVBoxLayout();
  rbvl->addWidget(allPar);
  rbvl->addWidget(markPar);
  rbvl->addWidget(noMarkPar);

  HideApplyBox* hab = new HideApplyBox(this);
  connect(hab, SIGNAL(hide()),  this, SIGNAL(paramHide()));
  connect(hab, SIGNAL(apply()), this, SIGNAL(paramApply()));

  QVBoxLayout * vl = new QVBoxLayout(this,10);
  vl->addWidget(plb);
  vl->addWidget(pVal);
  vl->addLayout(rbvl);
  vl->addWidget(hab);

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
