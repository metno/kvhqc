#ifndef STATIONORDEREDMISSINGDATAMODEL_HH
#define STATIONORDEREDMISSINGDATAMODEL_HH

#include "internal/MissingList.hh"
#include <QStandardItemModel>


class StationOrderedMissingDataModel : public QStandardItemModel
{
    Q_OBJECT
public:
    StationOrderedMissingDataModel(const MissingList & missing, QObject *parent = 0);
};

#endif // STATIONORDEREDMISSINGDATAMODEL_HH
