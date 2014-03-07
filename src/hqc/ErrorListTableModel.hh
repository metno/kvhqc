// -*- c++ -*-

#ifndef ERRORLISTTABLE_H
#define ERRORLISTTABLE_H

#include "common/AnalyseErrors.hh"
#include "common/EditAccess.hh"
#include "common/ModelAccess.hh"

#include <QtCore/QAbstractTableModel>

#include <vector>

class ErrorListTableModel : public QAbstractTableModel
{ Q_OBJECT;

public:
  ErrorListTableModel(EditAccessPtr eda, ModelAccessPtr mda,
      const Errors::Sensors_t& sensors, const TimeRange& limits, bool errorsForSalen);
  ~ErrorListTableModel();

  enum EDIT_COLUMNS {
    COL_STATION_ID = 0,
    COL_STATION_NAME,
    COL_STATION_WMO,
    COL_OBS_TIME,
    COL_OBS_PARAM,
    COL_OBS_TYPEID,
    COL_OBS_ORIG,
    COL_OBS_CORR,
    COL_OBS_MODEL,
    COL_OBS_FLAGS,
    NCOLUMNS
  };

  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  void showSameStation(int stationID);
  EditDataPtr mem4Row(int row) const;

  //! find row for sensortime, returns -1 if not found
  int findSensorTime(const SensorTime& st);

private:
  void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

private:
  EditAccessPtr mDA;
  ModelAccessPtr mMA;
  Errors::Sensors_t mSensors;
  TimeRange mTimeLimits;
  Errors::Errors_t mErrorList;
  bool mErrorsForSalen;
  int mShowStation;
};

#endif
