/* -*- c++ -*-

HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2013 met.no

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

#include "connect2stinfosys.h"
#include "hqcmain.h"

#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <Qt3Support/Q3Table>

#include <list>
#include <set>
#include <vector>

namespace Ui {
class ListDialog;
}

class ItemCheckBox : public QCheckBox
{ Q_OBJECT
public:
    ItemCheckBox(QString label, QString item, QWidget* parent=0)
        : QCheckBox(label, parent), mItem(item) { }
    QString getItem() const { return mItem; }
private Q_SLOTS:
    void clicked();
Q_SIGNALS:
    void clicked(QString item);
private:
    QString mItem;
};

class ListDialog : public QDialog
{ Q_OBJECT;
public:
    ListDialog(HqcMainWindow* parent);

    QDateTime getStart();
    QDateTime getEnd();
    void setEnd(const QDateTime& e);

    std::vector<int> getSelectedStations();

    QStringList getSelectedStationTypes();
    void setSelectedStationTypes(const QStringList& stationTypes);
    QStringList getSelectedCounties();
    void setSelectedCounties(const QStringList& c);
    bool showSynop() const;
    bool showPrioritized() const;
    
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

private Q_SLOTS:
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

    void prepareStationSelectionDialog();
    void showStationSelectionDialog();
    void selectAllStations();

Q_SIGNALS:
    void ListHide();
    void ListApply();
    void fromTimeChanged(const QDateTime&);
    void toTimeChanged(const QDateTime&);

private:
    std::auto_ptr<Ui::ListDialog> ui;
    class StationSelection* statSelect;
};

class StationTable : public Q3Table
{   Q_OBJECT
public:
    StationTable(QWidget* parent=0);
    void setData(const listStat_l& listStat, const QStringList& stationTypes, const QStringList& counties, bool web, bool pri);
    void sortColumn( int col, bool ascending, bool wholeRows );
private:
    QString getEnvironment(const int envID, const std::set<int>& typeIDs);
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
                     QWidget* parent);

    std::vector<int> getSelectedStations();

public Q_SLOTS:
    void doSelectAllStations();

private Q_SLOTS:
    void tableCellClicked(int, int, int, const QPoint&);
    void tableCellClicked(int, int);
    void tableCellClicked();

Q_SIGNALS:
  void stationAppended(QString);
  void stationRemoved(QString);

private:
    void selectOrDeselectStation(int row);

private:
    std::set<int> mSelectedStations;
};

class StTableItem : public Q3TableItem{
public:
  StTableItem( Q3Table *t, EditType et, const QString &txt ) :
    Q3TableItem( t, et, txt ) {}
  QString key() const;
};
#endif
