/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2012-2018 met.no

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


#ifndef WATCHRR_STATIONDIALOG_HH
#define WATCHRR_STATIONDIALOG_HH

#include "common/Sensor.hh"
#include "common/TimeSpan.hh"
#include "common/TypeIdModel.hh"

#include <QDialog>

#include <memory>

class TimeSpanControl;
class ObsPgmRequest;
namespace kvalobs {
class kvObsPgm;
}
namespace Ui {
class DialogStation;
}

class StationDialog : public QDialog
{ Q_OBJECT;
public:
  StationDialog(const Sensor& sensor, const TimeSpan& time, QWidget* parent=0);
  StationDialog(QWidget* parent=0);
  virtual ~StationDialog();
                            
  const Sensor& selectedSensor() const
    { return mSensor; }

  virtual TimeSpan selectedTime() const;

  virtual bool valid() const;

protected:
  virtual int acceptThisObsPgm(const kvalobs::kvObsPgm& op) const;

protected Q_SLOTS:
  virtual void onEditStation();
  virtual void onEditTime();
  virtual void onSelectType(int);
  void onObsPgmDone();

protected:
  void init();
  void updateStationInfoText();
  virtual bool checkStation();
  virtual bool checkType();
  virtual void updateTypeList();
  virtual void enableOk();

protected:
  Sensor mSensor;
  std::unique_ptr<Ui::DialogStation> ui;
  TimeSpanControl* mTimeControl;
  std::unique_ptr<TypeIdModel> mTypesModel;
  ObsPgmRequest* mObsPgmRequest;
};

#endif // WATCHRR_STATIONDIALOG_HH
