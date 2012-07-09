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
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <Q3HBoxLayout>
#include <QLabel>

IdentDialog::IdentDialog(QWidget* parent,const char* name, bool modal): QDialog(parent, name, modal) {  
  setCaption(tr("Innlogging"));

  Q3VBoxLayout* vl = new Q3VBoxLayout(this,10);

  nameEdit = new QLineEdit(this);
  nmlb = new QLabel(nameEdit,tr("Navn:   "),this);
  Q3HBoxLayout* nameLayout = new Q3HBoxLayout();
  nameLayout->addWidget(nmlb, 10);
  nameLayout->addWidget(nameEdit, 10);

  passEdit = new QLineEdit(this);
  passEdit->setEchoMode(QLineEdit::Password);
  pslb = new QLabel(passEdit,tr("Passord:"),this);
  Q3HBoxLayout* passLayout = new Q3HBoxLayout();
  passLayout->addWidget(pslb, 10);
  passLayout->addWidget(passEdit, 10);

  okbt = new QPushButton(tr("OK"), this);
  okbt->setGeometry(20, 620, 90, 30);
  okbt->setFont(QFont("Arial", 9));
  okbt->setDefault(true);
  
  cnsl = new QPushButton(tr("Avbryt"), this);
  cnsl->setGeometry(120, 620, 90, 30);
  cnsl->setFont(QFont("Arial", 9));

  Q3HBoxLayout* buttonLayout = new Q3HBoxLayout();
  buttonLayout->addWidget(okbt, 10);
  buttonLayout->addWidget(cnsl, 10);

  connect(okbt, SIGNAL(clicked()), this, SLOT( accept()));
  connect(cnsl, SIGNAL(clicked()), this, SLOT( reject()));

  Q3VBox* vb = new Q3VBox(this);

  QLabel* lb1 = new QLabel(tr("Skriv brukernavn og passord\nForeløpig nok å trykke OK"), vb);
  lb1->setMaximumHeight( lb1->sizeHint().height() * 2 );
  lb1->setFrameStyle( Q3Frame::Panel | Q3Frame::Sunken );
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
