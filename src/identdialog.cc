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
#include "identdialog.h"

IdentDialog::IdentDialog(QWidget* parent,const char* name, bool modal): QDialog(parent, name, modal) {  
  setCaption("Innlogging");

  QVBoxLayout* vl = new QVBoxLayout(this,10);

  nameEdit = new QLineEdit(this);
  nmlb = new QLabel(nameEdit,"Navn:   ",this);
  QHBoxLayout* nameLayout = new QHBoxLayout();
  nameLayout->addWidget(nmlb, 10);
  nameLayout->addWidget(nameEdit, 10);

  passEdit = new QLineEdit(this);
  passEdit->setEchoMode(QLineEdit::Password);
  pslb = new QLabel(passEdit,"Passord:",this);
  QHBoxLayout* passLayout = new QHBoxLayout();
  passLayout->addWidget(pslb, 10);
  passLayout->addWidget(passEdit, 10);

  okbt = new QPushButton("OK", this);
  okbt->setGeometry(20, 620, 90, 30);
  okbt->setFont(QFont("Arial", 9));
  okbt->setDefault(true);
  
  cnsl = new QPushButton("Avbryt", this);
  cnsl->setGeometry(120, 620, 90, 30);
  cnsl->setFont(QFont("Arial", 9));

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(okbt, 10);
  buttonLayout->addWidget(cnsl, 10);

  connect(okbt, SIGNAL(clicked()), this, SLOT( accept()));
  connect(cnsl, SIGNAL(clicked()), this, SLOT( reject()));

  QVBox* vb = new QVBox(this);

  QLabel* lb1 = new QLabel("Skriv brukernavn og passord\nForeløpig nok å trykke OK", vb);
  lb1->setMaximumHeight( lb1->sizeHint().height() * 2 );
  lb1->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  lb1->setFont( QFont("system",12,QFont::Bold) );

  vl->addWidget(vb);
  vl->addLayout(nameLayout);
  vl->addLayout(passLayout);
  vl->addLayout(buttonLayout);
}

void IdentDialog::checkIdent() {
}

QString IdentDialog::getName() {
  return userName;
}

QString IdentDialog::getPassword() {
  return userName;
}
