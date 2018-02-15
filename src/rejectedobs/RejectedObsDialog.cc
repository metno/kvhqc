/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "RejectedObsDialog.hh"

#include "util/timeutil.hh"
#include "util/MiDateTimeEdit.hh"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

RejectedObsDialog::RejectedObsDialog(QWidget* parent)
  : QDialog(parent)
{
  setWindowIcon(QIcon("icons:rejectedobs.svg"));

  textLabel1 = new QLabel(this);
  textLabel2 = new QLabel(this);
  textLabel3 = new QLabel(this);

  QDateTime dtto = timeutil::nowWithMinutes0Seconds0();
  QDateTime dtfrom = dtto.addDays(-2);
  fromEdit = new MiDateTimeEdit(dtfrom,this);
  toEdit   = new MiDateTimeEdit(dtto,this);
  fromEdit->setMaximumDate(dtto.date());
  fromEdit->setMaximumTime(dtto.time());
  fromEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
  toEdit->setMinimumDate(dtfrom.date());
  toEdit->setMinimumTime(dtfrom.time());
  toEdit->setDisplayFormat("yyyy-MM-dd hh:mm");

  okButton = new QPushButton(this);
  okButton->setGeometry(20, 620, 90, 30);
  okButton->setFont(QFont("Arial", 9));

  cancelButton = new QPushButton(this);
  cancelButton->setGeometry(120, 620, 90, 30);
  cancelButton->setFont(QFont("Arial", 9));

  QVBoxLayout* topLayout = new QVBoxLayout(this);
  QHBoxLayout* fromLayout = new QHBoxLayout();
  QHBoxLayout* toLayout = new QHBoxLayout();
  QHBoxLayout* buttonLayout = new QHBoxLayout();

  fromLayout->addWidget(textLabel2);
  fromLayout->addWidget(fromEdit);
  toLayout->addWidget(textLabel3);
  toLayout->addWidget(toEdit);
  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(cancelButton);

  topLayout->addWidget(textLabel1);
  topLayout->addLayout(fromLayout);
  topLayout->addLayout(toLayout);
  topLayout->addLayout(buttonLayout);

  retranslateUi();

  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(okButton,     SIGNAL(clicked()), this, SLOT(accept()));
}

void RejectedObsDialog::retranslateUi()
{
  setWindowTitle(tr("RejectDecode"));
  textLabel1->setText(tr("Select time range for reject list"));
  textLabel2->setText(tr("From"));
  textLabel3->setText(tr("To"));
  okButton->setText(tr("OK"));
  cancelButton->setText(tr("Cancel"));
}

void RejectedObsDialog::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QDialog::changeEvent(event);
}

TimeSpan RejectedObsDialog::getTimeSpan() const
{
  const timeutil::ptime f = timeutil::from_QDateTime(fromEdit->dateTime());
  const timeutil::ptime t = timeutil::from_QDateTime(toEdit  ->dateTime());
  return TimeSpan(f, t);
}
