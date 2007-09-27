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
#ifndef IDENTDIALOG_H
#define IDENTDIALOG_H

#include <qapplication.h>
#include <stdlib.h>
#include <iostream>
#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qlayout.h>

using namespace std;

class IdentDialog : public QDialog {
  Q_OBJECT

private:
  QString userName;
  QString passWord;
public:
  IdentDialog(QWidget*, const char*, bool);

  QLineEdit* nameEdit;
  QLineEdit* passEdit;

  QLabel* nmlb;
  QLabel* pslb;

  QPushButton* okbt;
  QPushButton* cnsl;

  QString getName();
  QString getPassword();

public slots:
  void checkIdent();
};

#endif
