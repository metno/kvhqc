/* -*- c++ -*-
   HQC - Free Software for Manual Quality Control of Meteorological Observations

   Copyright (C) 2013-2014 met.no

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

#ifndef ERRORLIST_H
#define ERRORLIST_H

#include "common/ObsAccess.hh"
#include "common/ModelAccess.hh"
#include "common/NavigateHelper.hh"
#include "util/BusyLabel.hh"

#include <QtGui/QWidget>

#include <memory>

QT_BEGIN_NAMESPACE
class QItemSelection;
class QSettings;
QT_END_NAMESPACE

class ErrorListModel;
class ErrorSearchDialog;
class Ui_ErrorList;

class ErrorList : public QWidget
{ Q_OBJECT;
public:
  ErrorList(QWidget* parent=0);
  ~ErrorList();

  void saveSettings(QSettings& settings);
  void restoreSettings(QSettings& settings);

public Q_SLOTS:
  void navigateTo(const SensorTime&);

Q_SIGNALS:
  void signalNavigateTo(const SensorTime&);

private Q_SLOTS:
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void onBeginDataChange();
  void onEndDataChange();

  void onButtonSearch();
  void onButtonExpand();
  void onButtonForget();

private:
  void setSensorsAndTimes(const Sensor_v& sensors, const TimeSpan& time);
  ObsData_p getSelectedObs() const;
  void resizeHeaders();
  void showSameStation();
  void signalStationSelected();
  void updateModel(const Sensor_v& sensors, const TimeSpan& time);

private:
  std::auto_ptr<Ui_ErrorList> ui;
  ErrorSearchDialog* mDialog;

  ObsAccess_p mDA;
  ModelAccess_p mMA;
  NavigateHelper mNavigate;
  bool mErrorsForSalen;

  std::auto_ptr<ErrorListModel> mItemModel;
};

#endif // ERRORLIST_H
