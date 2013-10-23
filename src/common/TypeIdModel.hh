
#ifndef common_TypeIdModel_hh
#define common_TypeIdModel_hh 1

#include <QtCore/QAbstractListModel>
#include <vector>

class TypeIdModel : public QAbstractListModel {
public:
  TypeIdModel(const std::vector<int>& typeIds);

  int rowCount(const QModelIndex&) const
    { return mTypeIds.size(); }

  QVariant data(const QModelIndex& index, int role) const;

  const std::vector<int>& typeIds() const
    { return mTypeIds; }

  void addTypeOverride(int typeId, const QString& format, const QString& label = "");

private:
  std::vector<int> mTypeIds;
  typedef std::pair<QString,QString> TypeOverrideData_t;
  typedef std::map<int, TypeOverrideData_t> TypeOverrides_t;
  TypeOverrides_t mTypeOverrides;
};

#endif // common_TypeIdModel_hh
