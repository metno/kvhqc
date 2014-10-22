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
  class CorrectedOrdering : public SortedBuffer::Ordering {
  public:
    CorrectedOrdering(bool a) : ascending(a) { }

    bool compare(ObsData_p a, ObsData_p b) const;
    bool compare(const SensorTime& a, const SensorTime& b) const;
    
  private:
    int compareCorrected(float a, float b) const;
    
  private:
    bool ascending;
  };

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

  ObsData_p getObs(int row) const;

private Q_SLOTS:
  void onBufferChangeBegin();
  void onBufferChangeEnd();

private:
  void updateCountShown();

private:
  EditAccess_p mDA;
  SortedBuffer_p mBuffer;
  int mCountShown;
};

#endif
