
#ifndef NEIGHBORTABLEMODEL_HH
#define NEIGHBORTABLEMODEL_HH 1

#include "ObsTableModel.hh"
#include <vector>

class NeighborTableModel : public ObsTableModel
{
public:
    NeighborTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time);
    virtual QVariant columnHeader(int section, Qt::Orientation orientation, int role) const;

private:
    std::vector<Sensor> mNeighbors;
};

#endif /* NEIGHBORTABLEMODEL_HH */
