
#ifndef NEIGHBORTABLEMODEL_HH
#define NEIGHBORTABLEMODEL_HH 1

#include "ObsTableModel.hh"
#include <vector>

class NeighborTableModel : public ObsTableModel
{
public:
    NeighborTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time);

private:
    std::vector<Sensor> findNeighbors(const Sensor& sensor);
};

#endif /* NEIGHBORTABLEMODEL_HH */
