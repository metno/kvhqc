// -*- c++ -*-

#ifndef EXTREMESTABLE_H
#define EXTREMESTABLE_H

#include "common/EditAccess.hh"
#include "common/TimeBuffer.hh"

#include <QtCore/QAbstractTableModel>

#include <vector>

class ExtremesTableModel : public QAbstractTableModel
{
  Q_OBJECT;

public:
  ExtremesTableModel(EditAccess_p eda);
  ~ExtremesTableModel();

  void search(int paramid, const TimeSpan& time);

  enum EDIT_COLUMNS {
    COL_STATION_ID = 0,
    COL_STATION_NAME,
    COL_OBSTIME,
    COL_OBS_PARAM,
    COL_OBS_TYPEID,
    COL_OBS_ORIG,
    COL_OBS_CORR,
    COL_OBS_FLAGS,
    NCOLUMNS
  };

  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  ObsData_p getObs(int row) const
    { return mExtremes.at(row); }

private Q_SLOTS:
  void onBufferCompleted(const QString&);

private:
  EditAccess_p mDA;
  TimeBuffer_p mBuffer;
  ObsData_pv mExtremes;
};

#endif
