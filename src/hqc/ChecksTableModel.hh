// -*- c++ -*-

#ifndef HQC_CHECKSTABLEMODEL_HH
#define HQC_CHECKSTABLEMODEL_HH

#include "common/ObsData.hh"

#include <QAbstractTableModel>
#include <QStringList>

#include <vector>

class ChecksTableModel : public QAbstractTableModel
{
public:
  ChecksTableModel(QObject* parent=0);
  ~ChecksTableModel();
  
  int rowCount(const QModelIndex&) const;
  int columnCount(const QModelIndex&) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  
  void showChecks(ObsData_p obs);

private:
  void buildModel(ObsData_p obs);

private:
  QStringList mChecks, mExplanations;
};

#endif // HQC_CHECKSTABLEMODEL_HH
