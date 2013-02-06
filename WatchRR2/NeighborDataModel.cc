
#include "NeighborDataModel.hh"
#include "ColumnFactory.hh"
#include "NeighborHeader.hh"
#include "SensorHeader.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define NDEBUG
#include "debug.hh"

namespace Helpers {
std::vector<Sensor> findNeighbors(const Sensor& sensor, const TimeRange& time, int maxNeighbors);
}

namespace /* anonymous */ {
const int N_COLUMNS = 18;
const int columnPars[N_COLUMNS] = {
    kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
    kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
    kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
    kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD,
    kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD,
    kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD
};
const ColumnFactory::DisplayType columnTypes[N_COLUMNS] = {
    ColumnFactory::NEW_CORRECTED,   ColumnFactory::NEW_CORRECTED,   ColumnFactory::NEW_CORRECTED,
    ColumnFactory::NEW_CORRECTED,   ColumnFactory::NEW_CORRECTED,   ColumnFactory::NEW_CORRECTED,
    ColumnFactory::NEW_CORRECTED,   ColumnFactory::NEW_CORRECTED,   ColumnFactory::NEW_CORRECTED,
    ColumnFactory::NEW_CORRECTED,   ColumnFactory::NEW_CORRECTED,   ColumnFactory::NEW_CORRECTED,
    ColumnFactory::ORIGINAL,        ColumnFactory::ORIGINAL,        ColumnFactory::ORIGINAL,
    ColumnFactory::NEW_CONTROLINFO, ColumnFactory::NEW_CONTROLINFO, ColumnFactory::NEW_CONTROLINFO
};
const int columnTimeOffsets[N_COLUMNS] = {
    -18, -18, -18,
    -12, -12, -12,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0
};
} // namespace anonymous

NeighborDataModel::NeighborDataModel(EditAccessPtr da/*, ModelAccessPtr ma*/, const Sensor& sensor, const TimeRange& timeRange)
    : mDA(da)
    , mTimeRange(timeRange)
    , mTime(mTimeRange.t0())
    , mSensors(Helpers::findNeighbors(sensor, mTimeRange, 20))
{
    mItems.reserve(N_COLUMNS);
    mTimeOffsets.reserve(N_COLUMNS);

    for(int i=0; i<N_COLUMNS; ++i) {
        const Sensor s(sensor.stationId, columnPars[i], sensor.level, sensor.sensor, sensor.typeId);
        mItems.push_back(ColumnFactory::itemForSensor(da, s, columnTypes[i]));

        mTimeOffsets.push_back(boost::posix_time::hours(columnTimeOffsets[i]));
    }

    BOOST_FOREACH(const Sensor& s, mSensors)
        mDA->addSubscription(ObsSubscription(s.stationId, mTimeRange));
    mDA->obsDataChanged.connect(boost::bind(&NeighborDataModel::onDataChanged, this, _1, _2));
}

NeighborDataModel::~NeighborDataModel()
{
    mDA->obsDataChanged.disconnect(boost::bind(&NeighborDataModel::onDataChanged, this, _1, _2));
    BOOST_FOREACH(const Sensor& s, mSensors)
        mDA->removeSubscription(ObsSubscription(s.stationId, mTimeRange));
}

int NeighborDataModel::rowCount(const QModelIndex&) const
{
    return mSensors.size();
}

int NeighborDataModel::columnCount(const QModelIndex&) const
{
    return mItems.size();
}

Qt::ItemFlags NeighborDataModel::flags(const QModelIndex& index) const
{
    return getItem(index)->flags() & ~Qt::ItemIsEditable;
}

QVariant NeighborDataModel::data(const QModelIndex& index, int role) const
{
    return getItem(index)->data(getObs(index), role);
}

void NeighborDataModel::setTime(const timeutil::ptime& time)
{
    if (not mTimeRange.contains(time) or mTime == time)
        return;

    mTime = time;
    /*emit*/ dataChanged(createIndex(0,0), createIndex(mSensors.size()-1, mItems.size()-1));
    /*emit*/ timeChanged(mTime);
}

QVariant NeighborDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole or role == Qt::ToolTipRole) {
        if (orientation == Qt::Horizontal) {
            const Sensor s(-1, columnPars[section], -1, -1, -1);
            SensorHeader sh(s, SensorHeader::NEVER, SensorHeader::ALWAYS, columnTimeOffsets[section]);
            return sh.sensorHeader(mItems[section], orientation, role);
        } else if (role == Qt::ToolTipRole) {
            SensorHeader sh(mSensors[section], SensorHeader::ALWAYS, SensorHeader::NEVER, 0);
            return sh.sensorHeader(DataItemPtr(), orientation, role);
        } else {
            return NeighborHeader::headerData(mSensors[0].stationId, mSensors[section].stationId, orientation, role);
        }
    }
    return QVariant();
}

EditDataPtr NeighborDataModel::getObs(const QModelIndex& index) const
{
    Sensor sensor = mSensors[index.row()];
    sensor.paramId = columnPars[index.column()];
    return mDA->findE(SensorTime(sensor, getTime(index)));
}

void NeighborDataModel::onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr obs)
{
    LOG_SCOPE();
    const SensorTime st(obs->sensorTime());

    unsigned int row = 0;
    for(;row < mSensors.size(); row += 1)
        if (eq_Sensor()(st.sensor, mSensors[row]))
            break;
    if (row >= mSensors.size())
        return;

    unsigned int col = 0;
    for(;col < mTimeOffsets.size(); col += 1)
        if (st.time == mTime + mTimeOffsets[col])
            break;
    if (col >= mTimeOffsets.size())
        return;

    QModelIndex idx = createIndex(row, col);
    /*emit*/ dataChanged(idx, idx);
}

std::vector<int> NeighborDataModel::neighborStations() const
{
    std::vector<int> n;
    n.reserve(mSensors.size());
    BOOST_FOREACH(const Sensor& s, mSensors)
        n.push_back(s.stationId);
    return n;
}
