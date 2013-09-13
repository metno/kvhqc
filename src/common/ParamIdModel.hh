
#ifndef ParamIdModel_hh
#define ParamIdModel_hh 1

#include <QtCore/QAbstractListModel>
#include <vector>

class ParamIdModel : public QAbstractListModel {
public:
    ParamIdModel(const std::vector<int>& paramIds);

    int rowCount(const QModelIndex&) const
        { return mParamIds.size(); }

    QVariant data(const QModelIndex& index, int role) const;

    const std::vector<int>& parameterIds() const
        { return mParamIds; }

private:
    std::vector<int> mParamIds;
};

#endif // ParamIdModel_hh
