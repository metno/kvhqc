// -*- c++ -*-

#ifndef HQC_DATAHISTORYTABLEMODEL_HH
#define HQC_DATAHISTORYTABLEMODEL_HH

#include "common/QueryTaskHandler.hh"
#include "common/DataHistoryQueryTask.hh"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QStringList>

#include <vector>

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
  void onCompleted(const SensorTime&, const kvDataHistoryValues_v&, bool);

private:
  void dropTask();

private:
  QueryTaskHandler_p mKvalobsHandler;
  DataHistoryQueryTask *mTask;
  SensorTime mSensorTime;
  kvDataHistoryValues_v mHistory;
};

#endif // HQC_DATAHISTORYTABLEMODEL_HH
