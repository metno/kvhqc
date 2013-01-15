
#include "NeighborTableModel.hh"

#include "ColumnFactory.hh"
#include "Helpers.hh"
#include "KvStationBuffer.hh"

#include <kvalobs/kvObsPgm.h>
#include <kvcpp/KvApp.h>

#include <boost/foreach.hpp>

#define NDEBUG
#include "debug.hh"

NeighborTableModel::NeighborTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
    : ObsTableModel(da, time)
{
    setTimeInRows(false);

    const std::vector<Sensor> neighbors = findNeighbors(sensor);
    const int n = std::min(1000, static_cast<int>(neighbors.size()));
    for(int i=0; i<n; ++i) {
        DataColumnPtr oc = ColumnFactory::columnForSensor(da, neighbors[i], time, DataColumn::ORIGINAL);
        oc->setHeaderShowStation(true);
        addColumn(oc);
    }
}

namespace /* anonymous */ {
struct stations_by_distance : public std::binary_function<bool, kvalobs::kvStation, kvalobs::kvStation> {
    kvalobs::kvStation center;
    bool operator()(const kvalobs::kvStation& a, const kvalobs::kvStation& b) const {
        if (a.stationID() == b.stationID())
            return false;
        return distance(a) < distance(b);
    }
    float distance(const kvalobs::kvStation& n) const {
        return Helpers::distance(center.lon(), center.lat(), n.lon(), n.lat());
    }
};
} // anonymous namespace

std::vector<Sensor> NeighborTableModel::findNeighbors(const Sensor& sensor)
{
    std::vector<Sensor> neighbors;
    if (not kvservice::KvApp::kvApp) {
        std::cerr << "no KvApp, probably running a test program" << std::endl;
        return neighbors;
    }

    const std::list<kvalobs::kvStation>& stationsList = KvStationBuffer::instance()->allStations();

    std::vector<kvalobs::kvStation> stations;
    stations_by_distance ordering;
    BOOST_FOREACH(const kvalobs::kvStation& s, stationsList)
        if (s.stationID() == sensor.stationId) {
            ordering.center = s;
            break;
        }
    // FIXME handle station not found

    std::list<long int> stationIDs;
    BOOST_FOREACH(const kvalobs::kvStation& s, stationsList) {
        const int sid = s.stationID();
        if (sid < 60 or sid >= 100000)
            continue;
        if (ordering.distance(s) > 100 /*km*/)
            continue;
        stations.push_back(s);
        stationIDs.push_back(s.stationID());
        DBGV(s.stationID());
    }
    std::sort(stations.begin(), stations.end(), ordering);

    std::list<kvalobs::kvObsPgm> obs_pgm;
    if (not kvservice::KvApp::kvApp->getKvObsPgm(obs_pgm, stationIDs, false)) {
        std::cerr << "problem loading obs_pgm" << std::endl;
        return neighbors;
    }

    std::map<int, kvalobs::kvObsPgm> obsPgmForStationsWithRR24;
    BOOST_FOREACH (const kvalobs::kvObsPgm& op, obs_pgm) {
        if (op.paramID() == kvalobs::PARAMID_RR
            and (op.typeID() == 302 or op.typeID() == 402)
            and (op.kl06() or op.kl07() or op.collector()))
        {
            obsPgmForStationsWithRR24[op.stationID()] = op;
        }
    }

    BOOST_FOREACH(const kvalobs::kvStation& s, stations) {
        std::map<int,kvalobs::kvObsPgm>::const_iterator it = obsPgmForStationsWithRR24.find(s.stationID());
        if (it == obsPgmForStationsWithRR24.end())
            continue;
        const kvalobs::kvObsPgm& op = it->second;
        neighbors.push_back(Sensor(s.stationID(), kvalobs::PARAMID_RR, op.level(), 0, op.typeID()));
        DBGV(s.stationID());
    }
    return neighbors;
}
