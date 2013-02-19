/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id: dianashowdialog.cc 684 2011-04-12 11:55:00Z knutj $

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
#include "approvedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <iostream>

using namespace std;

ApproveDialog::ApproveDialog(std::vector<QString>& chList) : QDialog() 
{  
  //  setCaption(tr("Forkast tidsserie"));
  QLabel* discLabel = new QLabel( tr("Do you want to accept these data?"), this );
  resultWidget = new QListWidget(this);
  int n = chList.size();
  for(int i=0; i<n; i++ ){
    new QListWidgetItem(chList[i], resultWidget);
  }
  //  resultWidget->addItem(ch);

  okButton = new QPushButton(tr("Accept time series"), this);
  okButton->setGeometry(20, 620, 90, 30);
  okButton->setFont(QFont("Arial", 9));

  cancelButton = new QPushButton(tr("Cancel"), this);
  cancelButton->setGeometry(120, 620, 90, 30);
  cancelButton->setFont(QFont("Arial", 9));

  connect(okButton,     SIGNAL(clicked()), this, SLOT( accept()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT( reject()));

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(okButton, 10);
  buttonLayout->addWidget(cancelButton, 10);

  QVBoxLayout* mainLayout = new QVBoxLayout(this, 10);
  mainLayout->addWidget(discLabel);
  mainLayout->addWidget(resultWidget);
  mainLayout->addLayout(buttonLayout);
}
