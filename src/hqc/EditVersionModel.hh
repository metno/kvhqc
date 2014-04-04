
#ifndef EditVersionModel_hh
#define EditVersionModel_hh 1

#include "access/EditAccess.hh"

#include <QtCore/QAbstractItemModel>

class EditVersionModel : public QAbstractItemModel
{ Q_OBJECT;
public:
  EditVersionModel(EditAccess_p eda);
  ~EditVersionModel();

  virtual int columnCount(const QModelIndex& parent) const;
  virtual int rowCount(const QModelIndex& parent) const;

  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  virtual bool hasChildren(const QModelIndex & parent) const;
  virtual QModelIndex index(int row, int column, const QModelIndex & parent) const;
  virtual QModelIndex parent(const QModelIndex& index) const;

private:
  void onCurrentVersionChanged(int current, int highest);
  //void onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);
  void dump();

private:
  EditAccess_p mDA;

  //typedef std::vector<EditAccess::ChangedData_t> ChangeHistory_t;
  //ChangeHistory_t mHistory;
};

#endif // EditVersionModel_hh
