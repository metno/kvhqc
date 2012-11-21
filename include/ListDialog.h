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

#include "ui_listdialog.h"

class ListDialog : public QDialog, private Ui_ListDialog
{   Q_OBJECT

public:
    ListDialog(QWidget*);

    void hideAll();
    void showAll();

    QDateTime getStart();
    QDateTime getEnd();
    void setEnd(const QDateTime& e);

    QStringList getSelectedStationTypes();
    void setSelectedStationTypes(const QStringList& stationTypes);
    QStringList getSelectedCounties();
    void setSelectedCounties(const QStringList& c);
    bool showSynop() const { return webReg->isChecked(); }
    bool showPrioritized() const { return priReg->isChecked(); }
    
private:
    void uncheckTypes();
    void checkTypes(const char* these[]);

private:
    std::list<ItemCheckBox*> mStationTypes;
    QCheckBox* allType;

    ItemCheckBox* oslCoun;
    ItemCheckBox* akeCoun;
    ItemCheckBox* ostCoun;
    ItemCheckBox* hedCoun;
    ItemCheckBox* oppCoun;
    ItemCheckBox* busCoun;
    ItemCheckBox* vefCoun;
    ItemCheckBox* telCoun;
    ItemCheckBox* ausCoun;
    ItemCheckBox* veaCoun;
    ItemCheckBox* rogCoun;
    ItemCheckBox* horCoun;
    ItemCheckBox* sogCoun;
    ItemCheckBox* morCoun;
    ItemCheckBox* sorCoun;
    ItemCheckBox* ntrCoun;
    ItemCheckBox* norCoun;
    ItemCheckBox* troCoun;
    ItemCheckBox* finCoun;
    ItemCheckBox* svaCoun;
    ItemCheckBox* allCoun;

public slots:
    void appendStatInListbox(QString);
    void removeStatFromListbox(QString);
    void removeAllStatFromListbox();

private slots:
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

signals:
    void ListHide();
    void ListApply();
    void selectStation();
    void fromTimeChanged(const QDateTime&);
    void toTimeChanged(const QDateTime&);
};

class StationTable : public Q3Table
{   Q_OBJECT
public:
    StationTable(QWidget* parent=0);
    void setData(const listStat_l& listStat, const QStringList& stationTypes, const QStringList& counties,
                 bool web, bool pri, ObsTypeList* otpList);
 bool findInTypes(ObsTypeList::iterator, int);
 QString getEnvironment(const int envID, ObsTypeList::iterator);
 void sortColumn( int col, bool ascending, bool wholeRows );
};

#include "ui_stationselection.h"

class StationSelection : public QDialog, public Ui_StationSelectionDialog
{   Q_OBJECT
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
