
#include "EditVersionModel.hh"

#include "Helpers.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define NDEBUG
#include "debug.hh"

EditVersionModel::EditVersionModel(EditAccessPtr eda)
    : mDA(eda)
{
    LOG_SCOPE("EditVersionModel");
    LOG4SCOPE_DEBUG(DBG1(mDA->currentVersion()) << DBG1(mDA->highestVersion()));
    mDA->currentVersionChanged.connect(boost::bind(&EditVersionModel::onCurrentVersionChanged, this, _1, _2));
    mDA->obsDataChanged       .connect(boost::bind(&EditVersionModel::onDataChanged,           this, _1, _2));
}

EditVersionModel::~EditVersionModel()
{
    mDA->obsDataChanged       .disconnect(boost::bind(&EditVersionModel::onDataChanged,           this, _1, _2));
    mDA->currentVersionChanged.disconnect(boost::bind(&EditVersionModel::onCurrentVersionChanged, this, _1, _2));
}

void EditVersionModel::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
    LOG_SCOPE("EditVersionModel");
    dump();
}

void EditVersionModel::onCurrentVersionChanged(int current, int highest)
{
    LOG_SCOPE("EditVersionModel");
    LOG4SCOPE_DEBUG(DBG1(current) << DBG1(highest));
    dump();
}

void EditVersionModel::dump()
{
    LOG_SCOPE("EditVersionModel");
    LOG4SCOPE_DEBUG(DBG1(mDA->currentVersion()) << DBG1(mDA->highestVersion()));
    /*emit*/ beginResetModel();
    mHistory = ChangeHistory_t();
    for(int v=1; v<=mDA->highestVersion(); ++v) {
        mHistory.push_back(mDA->versionChanges(v));
        const std::vector<EditDataPtr> changes = mHistory.back();
        LOG4SCOPE_DEBUG("changes for version " << v << " from " << mDA->versionTimestamp(v) << ":");
        BOOST_FOREACH(EditDataPtr obs, changes)
            LOG4SCOPE_DEBUG("   " << obs->sensorTime() << " c=" << obs->corrected(v) << " f=" << obs->controlinfo(v).flagstring());
    }
    /*emit*/ endResetModel();
}

int EditVersionModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

int EditVersionModel::rowCount(const QModelIndex& parent) const
{
    if (not parent.isValid())
        return mHistory.size();
    const qint64 internalId = parent.internalId();
    if (internalId >= 0)
        return mHistory.at(internalId).size();
    else
        return mHistory.size();
}

QVariant EditVersionModel::data(const QModelIndex& index, int role) const
{
    const qint64 internalId = index.internalId();

    if (role == Qt::DisplayRole) {
        if (internalId >= 0) {
            const EditDataPtr obs = mHistory.at(internalId).at(index.row());
            const SensorTime& st = obs->sensorTime();
            return tr("%1@%2: %3 corr=%4")
                .arg(st.sensor.stationId)
                .arg(QString::fromStdString(timeutil::to_iso_extended_string(st.time)))
                .arg(st.sensor.paramId)
                .arg(kvalobs::formatValue(obs->corrected(internalId+1)));
        } else {
            const timeutil::ptime& t = mDA->versionTimestamp(index.row()+1);
            return tr("Change at %1").arg(QString::fromStdString(timeutil::to_iso_extended_string(t)));
        }
    } else if (role == Qt::ForegroundRole) {
        const int version = 1 + ((internalId >= 0) ? internalId : index.row());
        if (version > mDA->currentVersion())
            return Qt::lightGray;
    }
    return QVariant();
}

bool EditVersionModel::hasChildren(const QModelIndex& index) const
{
    return (not index.isValid()) or (index.internalId() < 0);
}

QModelIndex EditVersionModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column == 0) {
        if (not parent.isValid()) {
            if (row < mHistory.size())
                return createIndex(row, column, -1);
        } else {
            if (row < mHistory.at(parent.row()).size())
                return createIndex(row, column, parent.row());
        }
    }
    return QModelIndex();
}

QModelIndex EditVersionModel::parent(const QModelIndex& index) const
{
    const qint64 internalId = index.internalId();
    if (internalId >= 0)
        return createIndex(internalId, 0, -1);
    else
        return QModelIndex();
}
