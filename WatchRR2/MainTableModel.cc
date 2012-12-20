
#include "MainTableModel.hh"

#include "ColumnFactory.hh"
#include "Helpers.hh"
#include "ModelColumn.hh"

#include <boost/make_shared.hpp>

#define NDEBUG
#include "debug.hh"

namespace /* anonymous */ {
const int N_COLUMNS = 18;
const int columnPars[N_COLUMNS] = {
    kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
    kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
    kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
    kvalobs::PARAMID_RR, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD,
    kvalobs::PARAMID_RR, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD,
    kvalobs::PARAMID_RR, kvalobs::PARAMID_SA, kvalobs::PARAMID_SD
};
const DataColumn::DisplayType columnTypes[N_COLUMNS] = {
    DataColumn::NEW_CORRECTED,   DataColumn::NEW_CORRECTED,   DataColumn::NEW_CORRECTED,
    DataColumn::NEW_CORRECTED,   DataColumn::NEW_CORRECTED,   DataColumn::NEW_CORRECTED,
    DataColumn::NEW_CORRECTED,   DataColumn::NEW_CORRECTED,   DataColumn::NEW_CORRECTED,
    DataColumn::NEW_CORRECTED,   DataColumn::NEW_CORRECTED,   DataColumn::NEW_CORRECTED,
    DataColumn::ORIGINAL,        DataColumn::ORIGINAL,        DataColumn::ORIGINAL,
    DataColumn::NEW_CONTROLINFO, DataColumn::NEW_CONTROLINFO, DataColumn::NEW_CONTROLINFO
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

MainTableModel::MainTableModel(EditAccessPtr da, ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time)
    : ObsTableModel(da, time)
{
    for(int i=0; i<N_COLUMNS; ++i) {
        const Sensor s(sensor.stationId, columnPars[i], sensor.level, sensor.sensor, sensor.typeId);
        DataColumnPtr oc = ColumnFactory::columnForSensor(da, s, columnTypes[i]);
        oc->setHeaderShowStation(false);
        if (columnTimeOffsets[i] != 0)
            oc->setTimeOffset(boost::posix_time::hours(columnTimeOffsets[i]));
        if (i == getRR24Column()) {
            mRR24EditTime = boost::make_shared<EditTimeColumn>(oc);
            addColumn(mRR24EditTime);
        } else {
            addColumn(oc);
        }
    }

    if (ma) {
        ModelColumnPtr mc = ColumnFactory::columnForSensor(ma, sensor);
        mc->setHeaderShowStation(false);
        addColumn(mc);
    }
}

int MainTableModel::getRR24Column() const
{
    // must match columns as listed above
    return 9;
}

void MainTableModel::setRR24TimeRange(const TimeRange& tr)
{
    mRR24EditTime->setEditableTime(tr);
}
