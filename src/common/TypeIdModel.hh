
#ifndef TypeIdModel_hh
#define TypeIdModel_hh 1

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

private:
    std::vector<int> mTypeIds;
};

#endif // TypeIdModel_hh
