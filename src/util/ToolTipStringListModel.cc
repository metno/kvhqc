
#include "ToolTipStringListModel.hh"

#define MILOGGER_CATEGORY "kvhqc.ToolTipStringListModel"
#include "util/HqcLogging.hh"

void ToolTipStringListModel::setToolTipList(const QStringList& ttl)
{
  beginResetModel();
  mToolTips = ttl;
  if (rowCount() != mToolTips.size()) {
// FIXME this gives lots of warnings for code parameters without short codes
//        HQC_LOG_ERROR("have " << rowCount() << " items but "
//                      << mToolTips.size() << " tooltips: [" << mToolTips.join(",") << "]");
  }
  endResetModel();
}

QVariant ToolTipStringListModel::data(const QModelIndex& index, int role) const
{
  if (role != Qt::ToolTipRole)
    return QStringListModel::data(index, role);
  if (index.row() >= mToolTips.size()) {
    HQC_LOG_ERROR("tooltip for item " << index.row() << " requested, but only "
        << mToolTips.size() << " tooltips: [" << mToolTips.join(",") << "]");
    return QVariant();
  }
  return mToolTips.at(index.row());
}
