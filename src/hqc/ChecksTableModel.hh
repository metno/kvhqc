// -*- c++ -*-

#ifndef HQC_CHECKSTABLEMODEL_HH
#define HQC_CHECKSTABLEMODEL_HH

#include "access/ObsAccess.hh"
#include "access/ObsData.hh"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QStringList>

#include <vector>

class ChecksTableModel : public QAbstractTableModel
{ Q_OBJECT;
public:
  ChecksTableModel(ObsAccess_p da);
  ~ChecksTableModel();
  
  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  
public Q_SLOTS:
  virtual void navigateTo(const SensorTime&);

private:
  //void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

private:
  ObsAccess_p mDA;
  SensorTime mSensorTime;
  QStringList mChecks, mExplanations;
};

#endif // HQC_CHECKSTABLEMODEL_HH
