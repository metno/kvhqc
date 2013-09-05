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
#ifndef LISTDIALOG2_HH
#define LISTDIALOG2_HH

#include "ItemCheckBox.hh"
#include "ParamIdModel.hh"

#include <QtCore/QSettings>
#include <QtGui/QDialog>

#include <map>
#include <memory>
#include <set>
#include <vector>

class HqcMainWindow;
class StationSelection;

namespace Ui {
class ListDialog;
}

class ListDialog : public QDialog
{ Q_OBJECT;
public:
    ListDialog(HqcMainWindow* parent);
    ~ListDialog();
    
    QDateTime getStart();
    void setStart(const QDateTime& s);
    QDateTime getEnd();
    void setEnd(const QDateTime& e);
    
    QStringList getSelectedStationTypes();
    std::vector<int> getSelectedStations();
    std::vector<int> getSelectedParameters();
    std::set<int> getSelectedTimes();
    bool isSelectAllStationTypes() const;
    
    bool showSynop() const;
    bool showPrioritized() const;
                                
    void saveSettings(QSettings& settings);
    void restoreSettings(QSettings& settings);

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

    void regionEastAdd();
    void regionEastRemove();
    void regionWestAdd();
    void regionWestRemove();
    void regionTrondAdd();
    void regionTrondRemove();
    void regionNorthAdd();
    void regionNorthRemove();

    void webCheck();
    void priCheck();
    void allCounCheck();
    void allCounUnCheck();

    void setMaxDate(const QDate&);
    void setMinDate(const QDate&);
    void setMaxTime(const QTime&);
    void setMinTime(const QTime&);

    void prepareStationSelectionDialog();
    void showStationSelectionDialog();
    void selectAllStations();

    void showParamGroup(const QString& paramGroup);
    void selectParameters();
    void deselectParameters();
    void selectAllParameters();
    void deselectAllParameters();

    void selectAllTimes();
    void deselectAllTimes();
    void selectStandardTimes();
    void deselectStandardTimes();

    void onSaveSettings();
    void onRestoreSettings();

Q_SIGNALS:
    void ListHide();
    void ListApply();

private:
    void uncheckTypes();
    void checkTypes(const char* these[]);
    void regionEastToggle(bool);
    void regionWestToggle(bool);
    void regionTrondToggle(bool);
    void regionNorthToggle(bool);
    void enableButtons();

    void setupStationTab();
    void setupParameterTab();
    void setupClockTab();

    void setSelectedStationTypes(const QStringList& stationTypes);
    QStringList getSelectedCounties();
    void setSelectedCounties(const QStringList& c);

    void doSaveSettings(QSettings& settings);
    void doRestoreSettings(QSettings& settings);

private:
    std::auto_ptr<Ui::ListDialog> ui;
    std::auto_ptr<StationSelection> statSelect;

    std::vector<ItemCheckBox*> mStationTypes;
    QCheckBox* allType;

    std::vector<ItemCheckBox*> mCounties;
    ItemCheckBox* allCoun;

    QCheckBox* mClockCheckBoxes[24];

    std::map<QString, std::vector<int> > mParameterGroups;
    std::auto_ptr<ParamIdModel> mParamAvailableModel;
    std::auto_ptr<ParamIdModel> mParamSelectedModel;
};

#endif // LISTDIALOG2_HH
