
#include "NeighborTableModel.hh"
#include "ColumnFactory.hh"

#include <boost/foreach.hpp>

namespace Helpers {
std::vector<Sensor> findNeighbors(const Sensor& sensor, const TimeRange& time, int maxNeighbors);
}

NeighborTableModel::NeighborTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
    : ObsTableModel(da, time)
{
    const std::vector<Sensor> neighbors = Helpers::findNeighbors(sensor, time, 20);
    BOOST_FOREACH(const Sensor& s, neighbors) {
        DataColumnPtr oc = ColumnFactory::columnForSensor(da, s, time, ColumnFactory::ORIGINAL);
        oc->setHeaderShowStation(true);
        addColumn(oc);
    }
}
