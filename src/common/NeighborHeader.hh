
#ifndef COMMON_GUI_NEIGHBORHEADER_HH
#define COMMON_GUI_NEIGHBORHEADER_HH 1

#include <QAbstractTableModel>

class NeighborHeader {
public:
    static QVariant headerData(int ctr, int nbr, Qt::Orientation orientation, int role);
};

#endif /* COMMON_GUI_NEIGHBORHEADER_HH */
