// -*- c++ -*-

#ifndef MISSINGTABLE_H
#define MISSINGTABLE_H

#include "common/EditAccess.hh"

#include <QtCore/QAbstractTableModel>

#include <vector>

class MissingTableModel : public QAbstractTableModel
{
public:
  MissingTableModel(EditAccessPtr eda, const std::vector<SensorTime>& missing);
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

private:
  void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

private:
  EditAccessPtr mDA;
  std::vector<SensorTime> mMissing;
};

#endif
