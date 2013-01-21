
#ifndef NEIGHBORHEADER_HH
#define NEIGHBORHEADER_HH 1

#include <QtCore/QAbstractTableModel>

class NeighborHeader {
public:
    static QVariant headerData(int ctr, int nbr, Qt::Orientation orientation, int role);
};

#endif /* NEIGHBORHEADER_HH */
