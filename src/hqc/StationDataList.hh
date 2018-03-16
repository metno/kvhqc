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


#ifndef StationDataList_hh
#define StationDataList_hh 1

#include "TimespanDataList.hh"

class ObsPgmRequest;
class QCheckBox;

class StationDataList : public TimespanDataList
{
  Q_OBJECT

public:
  StationDataList(QWidget* parent=0);
  ~StationDataList();

protected:
  std::string viewType() const override;
  void doSensorSwitch() override;
  SensorTime sensorSwitch() const override;
  void updateModel() override;

private Q_SLOTS:
  void onObsPgmsComplete();

private:
  void addSensorColumn(const Sensor& s, ObsColumn::Type type);
  void addSensorColumns(Sensor_s& alreadyShown, const Sensor& add);

private:
  QCheckBox* mCheckAggregated;
  QCheckBox* mCheckAllTypeIds;
  QCheckBox* mCheckAllParamIds;
  ObsPgmRequest* mObsPgmRequest;
};

#endif // StationDataList_hh
