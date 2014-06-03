
#ifndef EditVersionModel_hh
#define EditVersionModel_hh 1

#include "common/EditAccess.hh"
#include <QtCore/QAbstractItemModel>
#include <vector>

class EditVersionModel : public QAbstractItemModel
{ Q_OBJECT;
public:
  enum EDIT_COLUMNS {
    COL_TIME = 0,
    COL_STATION,
    COL_SENSORNR,
    COL_LEVEL,
    COL_TYPEID,
    COL_PARAMID,
    COL_CORRECTED,
    COL_FLAGS,
    NCOLUMNS
  };

  EditVersionModel(EditAccess_p eda, QObject* parent);
  ~EditVersionModel();

  EditAccess_p editAccess() const
    { return mDA; }

  virtual int columnCount(const QModelIndex& parent) const;
  virtual int rowCount(const QModelIndex& parent) const;

  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  virtual bool hasChildren(const QModelIndex & parent) const;
  virtual QModelIndex index(int row, int column, const QModelIndex & parent) const;
  virtual QModelIndex parent(const QModelIndex& index) const;

private Q_SLOTS:
  void onCurrentVersionChanged(size_t current, size_t highest);

private:
  void emitDataChanged(int version, bool includeParent);

private:
  EditAccess_p mDA;

  struct Change {
    timeutil::ptime timestamp;
    ObsData_pv changed;
    Change(const timeutil::ptime& ts) : timestamp(ts) { }
  };
  typedef std::vector<Change> Change_v;

  // mHistory.size() == highest; mHistory[0] = version 1; mHistory[highest-1] = highest version
  Change_v mHistory;
};

#endif // EditVersionModel_hh
