#ifndef MISSINGDATAMODEL_HH
#define MISSINGDATAMODEL_HH

#include <QStandardItemModel>
#include "internal/MissingList.hh"

class MissingDataModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit MissingDataModel(const MissingList & missing, QObject *parent = 0);

    virtual ~MissingDataModel();
};

#endif // MISSINGDATAMODEL_HH
