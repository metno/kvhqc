
#include "NeighborTableModel.hh"

#include "ColumnFactory.hh"
#include "Helpers.hh"
#include "NeighborHeader.hh"

#include <boost/foreach.hpp>

#define NDEBUG
#include "w2debug.hh"

NeighborTableModel::NeighborTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
    : ObsTableModel(da, time)
{
    mNeighbors = Helpers::findNeighbors(sensor, time, 20);
    BOOST_FOREACH(const Sensor& s, mNeighbors) {
        DataColumnPtr oc = ColumnFactory::columnForSensor(da, s, time, ColumnFactory::ORIGINAL);
        addColumn(oc);
        
    }
}

QVariant NeighborTableModel::columnHeader(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::ToolTipRole)
        return ObsTableModel::columnHeader(section, orientation, role);

    return NeighborHeader::headerData(mNeighbors[0].stationId, mNeighbors[section].stationId, orientation, role);
}
