/* -*- c++ -*-

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

#include "timeutil.hh"

#include <QtGui/qdialog.h>
#include <Qt3Support/q3buttongroup.h>
#include <QtGui/qpushbutton.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qcheckbox.h>
#include <QtGui/qradiobutton.h>
#include <QtGui/QGroupBox>
#include <QtGui/qlabel.h>
#include <Qt3Support/q3table.h>
#include <Qt3Support/q3listbox.h>
#include <QtGui/qcombobox.h>
#include <QtGui/qlayout.h>
#include <QtCore/qdatetime.h>
#include <Qt3Support/Q3VBoxLayout>

typedef std::list<int>                                 TypeList;
typedef std::list<TypeList>                         ObsTypeList;

struct listStat_t {
    std::string name;    // listStatName
    int stationid;       // listStatNum
    float altitude;      // listStatHoh
    int environment;     // listStatType
    std::string fylke;   // listStatFylke
    std::string kommune; // listStatKommune
    std::string wmonr;   // listStatWeb
    std::string pri;     // listStatPri
    timeutil::ptime fromtime;
    timeutil::ptime totime;
    bool coast;
};
typedef std::list<listStat_t> listStat_l;

class ItemCheckBox : public QCheckBox
{ Q_OBJECT
public:
    ItemCheckBox(QString label, QString item, QWidget* parent=0)
        : QCheckBox(label, parent), mItem(item) { }
    QString getItem() const { return mItem; }
private slots:
    void clicked();
signals:
    void clicked(QString item);
private:
    QString mItem;
};

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

    std::list<ItemCheckBox*> mStationTypes;
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

  QRadioButton* priTypes;
  QRadioButton* allTypes;

public slots:
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

  void setMaxDate(const QDate&);
  void setMinDate(const QDate&);
  void setMaxTime(const QTime&);
  void setMinTime(const QTime&);

    QStringList getSelectedStationTypes();
    QStringList getSelectedCounties();

    void setSelectedStationTypes(const QStringList& stationTypes);

private:
    void uncheckTypes();
    void checkTypes(const char* these[]);

private:
  QLabel* stationLabel;
  QLabel* fromLabel;
  QLabel* toLabel;
  QLabel* parameterLabel;
  QComboBox* parameterCombo;
  QDateTimeEdit* fromTime;
  QDateTimeEdit* toTime;
  QLineEdit* fromEdit;
  QLineEdit* toEdit;
  QPushButton* stationSelect;
  Q3ListBox* stationNames;
  Q3VBoxLayout* topLayout;
  QString weatherElement;

signals:
  void ListHide();
  void ListApply();
  void selectStation();
  void fromTimeChanged(const QDateTime&);
  void toTimeChanged(const QDateTime&);
};

class StationTable : public Q3Table {
Q_OBJECT
public:
 StationTable(const listStat_l& listStat,
              const QStringList& stationTypes,
              const QStringList& counties,
	      bool,
	      bool,
	      ObsTypeList*,
	      QWidget*);
 bool findInTypes(ObsTypeList::iterator, int);
 QString getEnvironment(const int envID, ObsTypeList::iterator);
 void sortColumn( int col, bool ascending, bool wholeRows );
};

class StationSelection : public QDialog {
Q_OBJECT
private:
 QPushButton* selectionOK;
 QPushButton* selectAllStations;
 StationTable* stationTable;
public:
 StationSelection(const listStat_l& listStat,
                  const QStringList& stationTypes,
                  const QStringList& counties,
		  bool,
		  bool,
		  ObsTypeList*,
                  QWidget* parent);
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

class StTableItem : public Q3TableItem{
public:
  StTableItem( Q3Table *t, EditType et, const QString &txt ) :
    Q3TableItem( t, et, txt ) {}
  QString key() const;
};
#endif
