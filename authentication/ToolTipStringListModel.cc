
#include "ToolTipStringListModel.hh"

#define MILOGGER_CATEGORY "kvhqc.ToolTipStringListModel"
#include "HqcLogging.hh"

void ToolTipStringListModel::setToolTipList(const QStringList& ttl)
{
    mToolTips = ttl;
    if (rowCount() != mToolTips.size()) {
        METLIBS_LOG_ERROR("have " << rowCount() << " items but "
                      << mToolTips.size() << " tooltips: [" << mToolTips.join(",") << "]");
    }
}

QVariant ToolTipStringListModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::ToolTipRole)
        return QStringListModel::data(index, role);
    if (index.row() >= mToolTips.size()) {
        METLIBS_LOG_ERROR("tooltip for item " << index.row() << " requested, but only "
                      << mToolTips.size() << " tooltips: [" << mToolTips.join(",") << "]");
        return QVariant();
    }
    return mToolTips.at(index.row());
}
