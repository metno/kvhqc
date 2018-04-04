/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

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


#ifndef SimpleCorrections_hh
#define SimpleCorrections_hh 1

#include "common/TimeBuffer.hh"
#include "common/ModelBuffer.hh"
#include "common/DataItem.hh"

#include <QWidget>
#include <memory>

class ChecksTableModel;
class DataHistoryTableModel;
QT_BEGIN_NAMESPACE
class Ui_SingleObservation;
QT_END_NAMESPACE

class SimpleCorrections : public QWidget
{ Q_OBJECT;
public:
  SimpleCorrections(EditAccess_p eda, ModelAccess_p mda, QWidget* parent=0);
  ~SimpleCorrections();
  
public Q_SLOTS:
  virtual void navigateTo(const SensorTime&);
  
protected:
  virtual void changeEvent(QEvent *event);
  
private:
  void enableEditing();

private Q_SLOTS:
  void onAcceptOriginal();
  void onAcceptModel();
  void onAcceptCorrected();
  void onReject();
  void onQc2Toggled(bool);

  void onNewCorrected();
  void onStartEditor();

  void update();
  void onDataChanged();

  void onHistoryTableUpdated();

private:
  Sensor_s allSensors() const;

private:
  std::unique_ptr<Ui_SingleObservation> ui;
  EditAccess_p mDA;
  ModelAccess_p mMA;
  ModelBuffer_p mModelBuffer;
  TimeBuffer_p mObsBuffer;

  ChecksTableModel *mChecksModel;
  DataHistoryTableModel *mHistoryModel;
  DataItem_p mItemFlags;
  DataItem_p mItemOriginal;
  DataItem_p mItemCorrected;
  SensorTime mSensorTime;
};

#endif // SimpleCorrections_hh
