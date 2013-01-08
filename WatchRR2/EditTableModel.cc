
#include "EditTableModel.hh"

#include "AnalyseRR24.hh"
#include "ColumnFactory.hh"
#include "Helpers.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/make_shared.hpp>

#define NDEBUG
#include "debug.hh"

namespace /* anonymous */ {
enum Columns {
    RR_24_orig,
    RR_24_old,
    RR_24_new,
    AcceptReject,
    N_COLUMNS
};
} // namespace anonymous

EditTableModel::EditTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
    : ObsTableModel(da, time)
    , mSensor(sensor)
    , mRR24Codes(ColumnFactory::codesForParam(kvalobs::PARAMID_RR))
{
    const int nDays = mTime.days() + 1;
    for(int d=0; d<nDays; ++d) {
        const timeutil::ptime t = timeAtRow(d);
        DBGV(t);
        EditDataPtr obs = mDA->findE(SensorTime(mSensor, t));
        if( not obs ) {
            mNewValues.push_back(kvalobs::MISSING);
            mAcceptReject.push_back(RR24::AR_NONE);
        } else {
            mNewValues.push_back(obs->corrected());

            int ar = RR24::AR_NONE;
            const int fhqc = obs->controlinfo().flag(kvalobs::flag::fhqc);
            if( fhqc > 0 and fhqc != 4 )
                ar = (Helpers::is_rejected(obs) ? RR24::AR_REJECT : RR24::AR_ACCEPT);
            mAcceptReject.push_back(ar);
        }
    }

    addColumn(ColumnFactory::columnForSensor(mDA, mSensor, time, DataColumn::ORIGINAL));
    addColumn(ColumnFactory::columnForSensor(mDA, mSensor, time, DataColumn::NEW_CORRECTED));
    addColumn(ObsColumnPtr());
    addColumn(ObsColumnPtr());
}

Qt::ItemFlags EditTableModel::flags(const QModelIndex& index) const
{
    if( not getColumn(index.column()) )
        return Qt::ItemIsEnabled|Qt::ItemIsEditable;
    return (ObsTableModel::flags(index) & ~(Qt::ItemIsSelectable|Qt::ItemIsEditable));
}

QVariant EditTableModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row(), column = index.column();
    if( (role == Qt::DisplayRole or role == Qt::EditRole) and column == RR_24_new ) {
        return mRR24Codes->asText(mNewValues.at(row));
    } else if( (role == Qt::ToolTipRole or role == Qt::StatusTipRole) and column == RR_24_new ) {
        return mRR24Codes->asTip(mNewValues.at(row));
    } else if( (role == Qt::DisplayRole or role == Qt::EditRole) and column == AcceptReject ) {
        switch( mAcceptReject.at(row) ) {
        case RR24::AR_ACCEPT:
            return "acc";
        case RR24::AR_REJECT:
            return "rej";
        default:
            return "-";
        }
    }
    return ObsTableModel::data(index, role);
}

QVariant EditTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role == Qt::DisplayRole and orientation == Qt::Horizontal and not getColumn(section) ) {
        if( section == RR_24_new )
            return "RR_24\nnew";
        else if( section == AcceptReject )
            return "Acc./Rej.";
    }
    return ObsTableModel::headerData(section, orientation, role);
}

bool EditTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if( role != Qt::EditRole or getColumn(index.column()) )
        return false;

    LOG_SCOPE();
    const int row = index.row(), column = index.column();
    if( column == RR_24_new ) {
        try {
            const float rrNew = mRR24Codes->fromText(value.toString());

            const float rrOld = mNewValues.at(row);
            if( fabs(rrNew - rrOld) < 0.05 )
                return false;

            mNewValues.at(row) = rrNew;
            mAcceptReject.at(row) = RR24::AR_ACCEPT;
            
            // both value and accept/reject have changed
            Q_ASSERT(RR_24_new + 1 == AcceptReject);
            QModelIndex index2 = createIndex(row, AcceptReject);
            dataChanged(index, index2);
            return true;
        } catch (std::string& error) {
            return false;
        }
    } else if( column == AcceptReject ) {
        const QString t = value.toString();
        if( t == "r" or t == "rej" ) {
            if( mAcceptReject.at(row) == RR24::AR_REJECT )
                return false;
            mAcceptReject.at(row) = RR24::AR_REJECT;
            mNewValues.at(row) = kvalobs::REJECTED;

            // both value and accept/reject have changed
            Q_ASSERT(RR_24_new + 1 == AcceptReject);
            QModelIndex index1 = createIndex(row, RR_24_new);
            dataChanged(index1, index);
            return true;
        } else if( t == "a" or t == "acc" ) {
            if( mAcceptReject.at(row) == RR24::AR_ACCEPT )
                return false;
            if( mNewValues.at(row) != -1 and mNewValues.at(row) < 0 )
                return false;
            mAcceptReject.at(row) = RR24::AR_ACCEPT;
            dataChanged(index, index);
            return true;
        }
    }
    return false;
}

void EditTableModel::acceptAll()
{
    const int nRows = rowCount(QModelIndex());
    for(int row=0; row<nRows; ++row) {
        if( mAcceptReject.at(row) == RR24::AR_ACCEPT )
            continue;
        if( mAcceptReject.at(row) == RR24::AR_REJECT ) {
            EditDataPtr obs = mDA->findE(SensorTime(mSensor, timeAtRow(row)));
            if( not obs )
                continue;
            mNewValues.at(row) = obs->corrected();
        } else if( mNewValues.at(row) != -1 and mNewValues.at(row) < 0 )
            continue;
        mAcceptReject.at(row) = RR24::AR_ACCEPT;
    }

    Q_ASSERT(RR_24_new + 1 == AcceptReject);
    QModelIndex index0 = createIndex(0, RR_24_new);
    QModelIndex index1 = createIndex(nRows-1, AcceptReject);
    dataChanged(index0, index1);
}

void EditTableModel::rejectAll()
{
    const int nRows = rowCount(QModelIndex());
    for(int row=0; row<nRows; ++row) {
        if( mAcceptReject.at(row) == RR24::AR_REJECT )
            continue;
        mAcceptReject.at(row) = RR24::AR_REJECT;
        mNewValues.at(row) = kvalobs::REJECTED;
    }

    Q_ASSERT(RR_24_new + 1 == AcceptReject);
    QModelIndex index0 = createIndex(0, RR_24_new);
    QModelIndex index1 = createIndex(nRows-1, AcceptReject);
    dataChanged(index0, index1);
}
