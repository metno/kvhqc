// -*- c++ -*-

#ifndef HQC_CHECKSTABLEMODEL_HH
#define HQC_CHECKSTABLEMODEL_HH

#include "common/ObsAccess.hh"
#include "common/ObsData.hh"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QStringList>

#include <vector>

class ChecksTableModel : public QAbstractTableModel
{ Q_OBJECT;
public:
  ChecksTableModel(ObsAccessPtr da);
  ~ChecksTableModel();
  
  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  
  virtual void navigateTo(const SensorTime&);

private:
  void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

private:
  ObsAccessPtr mDA;
  SensorTime mSensorTime;
  QStringList mChecks, mExplanations;
};

#endif // HQC_CHECKSTABLEMODEL_HH
