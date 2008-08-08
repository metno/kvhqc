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
#ifndef LISTDIALOG_H
#define LISTDIALOG_H

#include <stdlib.h>
#include <iostream>
#include <qdialog.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qtable.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <miTimeSpinBox.h>

using namespace std;

typedef list<int>                                 TypeList;
typedef list<TypeList>                         ObsTypeList;

class ListDialog : public QDialog {
  Q_OBJECT
    friend class HqcMainWindow;
public:
  ListDialog(QWidget*);

  void hideAll();
  void showAll();

  QString getStart();
  QString getEnd();
  QString getWeatherElement();

  QCheckBox* twiType;
  QCheckBox* prcType;
  QCheckBox* aprType;
  QCheckBox* winType;
  QCheckBox* marType;
  QCheckBox* visType;

  QCheckBox* aaType;
  QCheckBox* afType;
  QCheckBox* alType;
  QCheckBox* avType;
  QCheckBox* aoType;
  QCheckBox* aeType;
  QCheckBox* mvType;
  QCheckBox* mpType;
  QCheckBox* mmType;
  QCheckBox* msType;
  QCheckBox* fmType;
  QCheckBox* nsType;
  QCheckBox* ndType;
  QCheckBox* noType;
  QCheckBox* piType;
  QCheckBox* ptType;
  QCheckBox* vsType;
  QCheckBox* vkType;
  QCheckBox* vmType;
  QCheckBox* allType;

  QCheckBox* oslCoun;
  QCheckBox* akeCoun;
  QCheckBox* ostCoun;
  QCheckBox* hedCoun;
  QCheckBox* oppCoun;
  QCheckBox* busCoun;
  QCheckBox* vefCoun;
  QCheckBox* telCoun;
  QCheckBox* ausCoun;
  QCheckBox* veaCoun;
  QCheckBox* rogCoun;
  QCheckBox* horCoun;
  QCheckBox* sogCoun;
  QCheckBox* morCoun;
  QCheckBox* sorCoun;
  QCheckBox* ntrCoun;
  QCheckBox* norCoun;
  QCheckBox* troCoun;
  QCheckBox* finCoun;
  QCheckBox* svaCoun;
  QCheckBox* allCoun;

  QCheckBox* ausReg;
  QCheckBox* vesReg;
  QCheckBox* troReg;
  QCheckBox* norReg;
  QCheckBox* webReg;
  QCheckBox* priReg;

public slots:
  void applyHideClicked();
  void chooseParameters(const QString &);
  void appendStatInListbox(QString);
  void removeStatFromListbox(QString);
  void removeAllStatFromListbox();
  void twiCheck();
  void prcCheck();
  void aprCheck();
  void winCheck();
  void visCheck();
  void marCheck();
  void otwiCheck();
  void oprcCheck();
  void oaprCheck();
  void owinCheck();
  void ovisCheck();
  void omarCheck();

  void ausCheck();
  void oausCheck();
  void vesCheck();
  void ovesCheck();
  void troCheck();
  void otroCheck();
  void norCheck();
  void onorCheck();
  void webCheck();
  void owebCheck();
  void priCheck();
  void opriCheck();
  void allCounCheck();
  void allCounUnCheck();

private:
  QLabel* stationLabel;
  QLabel* fromLabel;
  QLabel* toLabel;
  QLabel* parameterLabel;
  QComboBox* parameterCombo;
  miTimeSpinBox* fromTime;
  miTimeSpinBox* toTime;
  QLineEdit* fromEdit;
  QLineEdit* toEdit;
  QPushButton* sthide;
  QPushButton* hdnexcu;
  QPushButton* excu;
  //  QPushButton* parameterSelect;
  QPushButton* stationSelect;
  QListBox* stationNames;
  //  QGridLayout* grid;
  QVBoxLayout* topLayout;
  //  QString sct;
  //  QString spt;
  QString weatherElement;

signals:
  void ListHide();
  void ListApply();
  void selectStation();
  //  void selectParameter();
  void fromTimeChanged(const miutil::miTime&);
  void toTimeChanged(const miutil::miTime&);
};
/*
class ParameterTable : public QTable {
Q_OBJECT
public:
 ParameterTable(QStringList, int, int, QWidget*);
 void sortColumn( int col, bool ascending, bool wholeRows );
};

class ParameterSelection : public QWidget {
Q_OBJECT
private:
 QPushButton* selectionOK;
 ParameterTable* parameterTable;
public:
 ParameterSelection(QStringList, int);
 void showSelectedParameters(int);
 QStringList parList;
private slots:
 void tableCellClicked(int);
 void listSelectedParameters();
 //signals:
 //  void parameterAppended(QString);
 //  void parameterRemoved(QString);
 //  void parametersSelected(QStringList);
};
*/
class StationTable : public QTable {
Q_OBJECT
public:
 StationTable(QStringList, 
	      QStringList, 
	      QStringList, 
	      QStringList, 
	      QStringList, 
	      QStringList, 
	      QStringList, 
	      QStringList, 
	      int, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool, 
	      bool,
	      bool,
	      bool,
	      bool, 
	      int,
	      ObsTypeList*,
	      QWidget*);
 bool findInTypes(ObsTypeList::iterator, int);
 QString getEnvironment(QString, ObsTypeList::iterator);
 void sortColumn( int col, bool ascending, bool wholeRows );
};

class StationSelection : public QWidget {
Q_OBJECT
private:
 QPushButton* selectionOK;
 QPushButton* selectAllStations;
 StationTable* stationTable;
public:
 StationSelection(QStringList, 
		  QStringList, 
		  QStringList, 
		  QStringList, 
		  QStringList, 
		  QStringList, 
		  QStringList, 
		  QStringList, 
		  int, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool, 
		  bool,
		  bool,
		  bool, 
		  int,
		  ObsTypeList*);
 void showSelectedStation(int, int);
 QStringList stlist;
private slots:
  void tableCellClicked(int, int, int, const QPoint&);
  void tableCellClicked(int, int);
  void tableCellClicked();
  void listSelectedStations();
signals:
  void stationAppended(QString);
  void stationRemoved(QString);
  void stationsSelected(QStringList);
public slots:
 void showAllStations();
};
 
class StTableItem : public QTableItem{
public:
  StTableItem( QTable *t, EditType et, const QString &txt ) : 
    QTableItem( t, et, txt ) {}
  QString key() const;
};
#endif
