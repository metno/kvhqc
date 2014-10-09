// -*- c++ -*-

#ifndef MISSINGTABLE_H
#define MISSINGTABLE_H

#include "common/KvTypedefs.hh"
#include "common/QueryTaskHandler.hh"
#include "common/QueryTaskHelper.hh"
#include "common/Sensor.hh"
#include "common/TimeSpan.hh"

#include <QtCore/QAbstractTableModel>

class MissingTableModel : public QAbstractTableModel
{ Q_OBJECT;
public:
  MissingTableModel(QueryTaskHandler_p handler);
  ~MissingTableModel();

  enum EDIT_COLUMNS {
    COL_STATION_ID = 0,
    COL_STATION_NAME,
    COL_OBSTIME,
    COL_OBS_TYPEID,
    NCOLUMNS
  };

  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  const SensorTime& getSensorTime(int row) const
    { return mMissing.at(row); }

  void search(const TimeSpan& time, const hqc::int_s& typeIds);

private Q_SLOTS:
  void onQueryDone(SignalTask* task);

private:
  void dropTask();

private:
  QueryTaskHandler_p mKvalobsHandler;
  QueryTaskHelper *mTask;
  SensorTime_v mMissing;
};

#endif
