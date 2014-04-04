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

#ifndef ERRORLIST_H
#define ERRORLIST_H

#include "common/ObsAccess.hh"
#include "common/ModelAccess.hh"
#include "common/NavigateHelper.hh"

#include <QtGui/QTreeView>

#include <memory>

class ErrorListModel;

class ErrorList : public QTreeView
{ Q_OBJECT;
public:
  ErrorList(QWidget* parent=0);
  ~ErrorList();

  void setErrorsForSalen(bool errorsForSalen)
    { mErrorsForSalen = errorsForSalen; }

  virtual void setDataAccess(ObsAccess_p eda, ModelAccess_p mda);
  virtual void setSensorsAndTimes(const Sensor_v& sensors, const TimeSpan& time);

public Q_SLOTS:
  void navigateTo(const SensorTime&);

Q_SIGNALS:
  void signalNavigateTo(const SensorTime&);

private:
  ObsData_p getSelectedObs() const;
                                    
private Q_SLOTS:
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void onBeginDataChange();
  void onEndDataChange();

private:
  void resizeHeaders();
  void showSameStation();
  void signalStationSelected();
  void updateModel(const Sensor_v& sensors, const TimeSpan& time);

private:
  ObsAccess_p mDA;
  ModelAccess_p mMA;
  NavigateHelper mNavigate;
  bool mErrorsForSalen;

  std::auto_ptr<ErrorListModel> mItemModel;
};

#endif // ERRORLIST_H
