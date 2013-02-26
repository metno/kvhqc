
#include "RedistTableModel.hh"

#include "AnalyseRR24.hh"
#include "ColumnFactory.hh"
#include "Helpers.hh"
#include "mi_foreach.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/make_shared.hpp>

#define NDEBUG
#include "debug.hh"

RedistTableModel::RedistTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
    : ObsTableModel(da, time)
    , mSensor(sensor)
    , mRR24Codes(ColumnFactory::codesForParam(kvalobs::PARAMID_RR_24))
{
    addColumn(ColumnFactory::columnForSensor(mDA, mSensor, time, ColumnFactory::ORIGINAL));
    addColumn(ColumnFactory::columnForSensor(mDA, mSensor, time, ColumnFactory::NEW_CORRECTED));
    addColumn(ObsColumnPtr());

    const int nDays = mTime.days() + 1;
    for(int d=0; d<nDays; ++d) {
        EditDataPtr obs = mDA->findE(SensorTime(mSensor, timeAtRow(d)));
        if (not obs)
            mNewValues.push_back(kvalobs::MISSING);
        else
            mNewValues.push_back(obs->corrected());
    }
}

RedistTableModel::~RedistTableModel()
{
    LOG_SCOPE();
}

Qt::ItemFlags RedistTableModel::flags(const QModelIndex& index) const
{
    if (not getColumn(index.column()))
        return Qt::ItemIsEnabled|Qt::ItemIsEditable;
    return (ObsTableModel::flags(index) & ~(Qt::ItemIsSelectable|Qt::ItemIsEditable));
}

QVariant RedistTableModel::data(const QModelIndex& index, int role) const
{
    if ((role == Qt::DisplayRole or role == Qt::EditRole) and not getColumn(index.column())) {
        return mRR24Codes->asText(mNewValues.at(index.row()));
    } else if ((role == Qt::ToolTipRole or role == Qt::StatusTipRole) and not getColumn(index.column())) {
        return mRR24Codes->asTip(mNewValues.at(index.row()));
    } else {
        return ObsTableModel::data(index, role);
    }
}

QVariant RedistTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole and orientation == Qt::Horizontal and not getColumn(section))
        return "RR_24\nnew";
    return ObsTableModel::headerData(section, orientation, role);
}

bool RedistTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole or getColumn(index.column()))
        return false;

    try {
        const float rrNew = mRR24Codes->fromText(value.toString());

        const int row = index.row();
        const float rrOld = mNewValues.at(row);
        if (fabs(rrNew - rrOld) < 0.05)
            return false;

        mNewValues.at(row) = rrNew;
        dataChanged(index, index);
        return true;
    } catch (std::runtime_error&) {
        return false;
    }
}

float RedistTableModel::originalSum() const
{
    return RR24::calculateOriginalSum(mDA, mSensor, mTime);
}

float RedistTableModel::currentSum() const
{
    float sum = 0;
    mi_foreach(float v, mNewValues)
        if (v >= 0)
            sum += v;
    return sum;
}

bool RedistTableModel::hasManualChanges() const
{
    const int nDays = mTime.days() + 1;
    for (int d=0; d<nDays; ++d) {
        EditDataPtr obs = mDA->findE(SensorTime(mSensor, timeAtRow(d)));
        if (not obs and mNewValues.at(d) != kvalobs::MISSING)
            return true;
        else if (obs and not Helpers::float_eq()(mNewValues.at(d), obs->corrected()))
            return true;
    }
    return false;
}
