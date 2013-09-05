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
#include "TimeRange.hh"

#include <QtCore/QSettings>
#include <QtGui/QDialog>

#include <map>
#include <memory>
#include <set>
#include <vector>

class HqcMainWindow;
class StationSelection;
class TimeRangeControl;

namespace Ui {
class ListDialog;
}

class ListDialog : public QDialog
{ Q_OBJECT;
public:
    ListDialog(HqcMainWindow* parent);
    ~ListDialog();
    
    TimeRange getTimeRange() const;
    
    std::vector<int> getSelectedStations();
    std::vector<int> getSelectedParameters();
    
    void saveSettings(QSettings& settings);
    void restoreSettings(QSettings& settings);

private Q_SLOTS:
    void appendStatInListbox(QString);
    void removeStatFromListbox(QString);
    void removeAllStatFromListbox();

    void regionEastAdd();
    void regionEastRemove();
    void regionWestAdd();
    void regionWestRemove();
    void regionMidAdd();
    void regionMidRemove();
    void regionNorthAdd();
    void regionNorthRemove();

    void allCounCheck();
    void allCounUnCheck();

    void onSetRecentTimes();

    void prepareStationSelectionDialog();
    void showStationSelectionDialog();
    void selectAllStations();

    void showParamGroup(const QString& paramGroup);
    void selectParameters();
    void deselectParameters();
    void selectAllParameters();
    void deselectAllParameters();

    void onSaveSettings();
    void onRestoreSettings();

Q_SIGNALS:
    void ListHide();
    void ListApply();

private:
    void regionEastToggle(bool);
    void regionWestToggle(bool);
    void regionMidToggle(bool);
    void regionNorthToggle(bool);
    void enableButtons();

    void setupStationTab();
    ItemCheckBox* addCountyCheckbox(const QString& c, const char* cu, int& x, int& y);

    void setupParameterTab();

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
    ItemCheckBox* mCountyAll;

    std::map<QString, std::vector<int> > mParameterGroups;
    std::auto_ptr<ParamIdModel> mParamAvailableModel;
    std::auto_ptr<ParamIdModel> mParamSelectedModel;

  TimeRangeControl* mTimeControl;
};

#endif // LISTDIALOG2_HH
