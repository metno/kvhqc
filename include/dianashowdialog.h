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
#ifndef DIANASHOWDIALOG_H
#define DIANASHOWDIALOG_H

#include <stdlib.h>
#include <iostream>
#include <qdialog.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>

using namespace std;

class DianaShowDialog : public QDialog {
  Q_OBJECT
public:
  DianaShowDialog(QWidget*);
  //  ~DianaShowDialog();

  void hideAll();
  void showAll();
  void checkStandard();

  //  QCheckBox* enkType;
  //  QCheckBox* synType;

  //  QCheckBox* obsType;
  //  QCheckBox* modType;
  //  QCheckBox* calType;

  //  QRadioButton* alType;
  QButtonGroup* paraTyp;

  QButtonGroup* taGroup;
  QRadioButton* taType;
  QRadioButton* tamoType;
  QRadioButton* tameType;
  QRadioButton* tadiType;

  QButtonGroup* tdGroup;
  QRadioButton* tdType;
  QRadioButton* uumoType;
  QRadioButton* uuType;
  QRadioButton* uumeType;
  QRadioButton* uumiType;
  QRadioButton* uumaType;

  QButtonGroup* prGroup;
  QRadioButton* prType;
  QRadioButton* prmoType;
  QRadioButton* poType;
  QRadioButton* pomeType;
  QRadioButton* pomiType;
  QRadioButton* pomaType;
  QRadioButton* phType;
  QRadioButton* podiType;

  QButtonGroup* ppGroup;
  QRadioButton* ppType;
  QRadioButton* ppmoType;

  QButtonGroup* rrGroup;
  QRadioButton* rrType;
  QRadioButton* rrmoType;
  QRadioButton* rr1Type;
  QRadioButton* rr24Type;
  QRadioButton* rr24moType;
  QRadioButton* rr6Type;
  QRadioButton* rrprType;

  QButtonGroup* tnxGroup;
  QRadioButton* tn12Type;
  QRadioButton* tx12Type;
  QRadioButton* tnType;
  QRadioButton* txType;

  QButtonGroup* ddGroup;
  QRadioButton* ddType;
  QRadioButton* ddmoType;

  QButtonGroup* ffGroup;
  QRadioButton* ffType;
  QRadioButton* ffmoType;

  QButtonGroup* fxGroup;
  QRadioButton* fxType;
  QRadioButton* fx01Type;

  QButtonGroup* fgGroup;
  QRadioButton* fgType;
  QRadioButton* fg01Type;
  QRadioButton* fg10Type;

  QButtonGroup* saGroup;
  QRadioButton* saType;
  QRadioButton* sdType;
  QRadioButton* emType;

  QPushButton* sthide;
  QPushButton* hdnexcu;
  QPushButton* excu;
public slots:
  void applyHideClicked();
signals:
  void dianaShowHide();
  void dianaShowApply();
};
#endif
