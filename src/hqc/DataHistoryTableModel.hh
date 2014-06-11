// -*- c++ -*-

#ifndef HQC_DATAHISTORYTABLEMODEL_HH
#define HQC_DATAHISTORYTABLEMODEL_HH

#include "common/Code2Text.hh"
#include "common/kvDataHistoryValues.hh"
#include "common/QueryTaskHandler.hh"
#include "common/QueryTaskHelper.hh"
#include "common/Sensor.hh"

#include <QtCore/QAbstractTableModel>

class DataHistoryTableModel : public QAbstractTableModel
{ Q_OBJECT;
public:
  DataHistoryTableModel(QObject* parent=0);
  ~DataHistoryTableModel();
  
  int rowCount(const QModelIndex&) const;
  int columnCount(const QModelIndex&) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  
  void showHistory(const SensorTime& st);

private Q_SLOTS:
  void onQueryDone(SignalTask* task);

private:
  void dropTask();
  const kvDataHistoryValues_v& history() const
    { return mHistory; }

private:
  QueryTaskHandler_p mKvalobsHandler;
  QueryTaskHelper *mTask;
  kvDataHistoryValues_v mHistory;
  Code2TextCPtr mCode2Text;
};

#endif // HQC_DATAHISTORYTABLEMODEL_HH
