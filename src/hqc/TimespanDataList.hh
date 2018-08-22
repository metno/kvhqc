/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2014-2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef TimespanDataList_hh
#define TimespanDataList_hh 1

#include "DataList.hh"
#include "common/ObsColumn.hh"

class QMenu;
class QToolButton;
class QSignalMapper;

// ------------------------------------------------------------------------

class TimespanDataList : public DataList
{ Q_OBJECT
public:
  TimespanDataList(QWidget* parent=0);
  ~TimespanDataList();
  
protected:
  SensorTime sensorSwitch() const override;
  void doSensorSwitch() override;
  virtual void doSensorSwitchBegin();
  virtual void doSensorSwitchEnd();
  void setDefaultTimeSpan();
  void loadChangesXML(const QDomElement& doc_changes) override;
  void storeChangesXML(QDomElement& doc_changes) override;

  void retranslateUi() override;

  const TimeSpan& timeSpan() const
    { return mTimeLimits; }

protected Q_SLOTS:
  virtual void updateModel() = 0;

private Q_SLOTS:
  void onChangeStart(QObject*);
  void onChangeEnd(QObject*);
  void onResetTime();

private:
  QSignalMapper* mMapperStart;
  QSignalMapper* mMapperEnd;
  QMenu* mMenuTime;
  QMenu* mMenuStart;
  QMenu* mMenuEnd;
  QToolButton* mButtonTime;
  QList<QAction*> mTimeActions;
  QAction* mActionResetTime;

  TimeSpan mTimeLimits, mOriginalTimeLimits;
};

#endif // TimespanDataList_hh
