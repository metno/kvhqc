
#include "EditVersionModel.hh"

#include "common/HqcApplication.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.EditVersionModel"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {

const char* headers[EditVersionModel::NCOLUMNS] = {
  QT_TRANSLATE_NOOP("EditVersionModel", "Time"),
  QT_TRANSLATE_NOOP("EditVersionModel", "Stnr"),
  QT_TRANSLATE_NOOP("EditVersionModel", "Snr"),
  QT_TRANSLATE_NOOP("EditVersionModel", "Lvl"),
  QT_TRANSLATE_NOOP("EditVersionModel", "Type"),
  QT_TRANSLATE_NOOP("EditVersionModel", "Par"),
  QT_TRANSLATE_NOOP("EditVersionModel", "Corr."),
  QT_TRANSLATE_NOOP("EditVersionModel", "Flags")
};

const char* tooltips[EditVersionModel::NCOLUMNS] = {
  QT_TRANSLATE_NOOP("EditVersionModel", "Change time / Observation time"),
  QT_TRANSLATE_NOOP("EditVersionModel", "Station number"),
  QT_TRANSLATE_NOOP("EditVersionModel", "Sensor number"),
  QT_TRANSLATE_NOOP("EditVersionModel", "Level"),
  QT_TRANSLATE_NOOP("EditVersionModel", "Type"),
  QT_TRANSLATE_NOOP("EditVersionModel", "Parameter"),
  QT_TRANSLATE_NOOP("EditVersionModel", "New Corrected Value"),
  QT_TRANSLATE_NOOP("EditVersionModel", "New Flags")
};

} // namespace anonymous

EditVersionModel::EditVersionModel(EditAccess_p eda, QObject* parent)
  : QAbstractItemModel(parent)
  , mDA(eda)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(mDA->currentVersion()) << LOGVAL(mDA->highestVersion()));
  connect(eda.get(), SIGNAL(currentVersionChanged(size_t, size_t)),
      this, SLOT(onCurrentVersionChanged(size_t, size_t)));
}

EditVersionModel::~EditVersionModel()
{
}

void EditVersionModel::emitDataChanged(int version, bool includeParent)
{
  if (version < 1 or version >= (int)mHistory.size())
      return;

  const QModelIndex parent = index(version-1, 0, QModelIndex());
  if (includeParent)
    Q_EMIT dataChanged(parent, parent);
  Q_EMIT dataChanged(index(0, 0, parent), index(rowCount(parent)-1, columnCount(parent), parent));
}

void EditVersionModel::onCurrentVersionChanged(size_t current, size_t highest)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(mHistory.size()) << LOGVAL(current) << LOGVAL(highest));

  if (highest < mHistory.size()) {
    beginRemoveRows(QModelIndex(), highest, mHistory.size()-1);
    Change_v::iterator it = mHistory.begin() + highest;
    mHistory.erase(it, mHistory.end());
    endRemoveRows();
  } else if (highest > mHistory.size()) {
    beginInsertRows(QModelIndex(), mHistory.size(), highest-1);
    for (size_t c = mHistory.size()+1; c<=highest; ++c)
      mHistory.push_back(Change(mDA->versionTimestamp(c)));
    endInsertRows();
  }
  if (current >= 1) {
    const QModelIndex parent = index(current-1, 0, QModelIndex());
    const ObsData_pv changed = mDA->versionChanges(current);
    Change& c = mHistory.back();
    const size_t nc = c.changed.size();
    if (changed.size() < nc) {
      beginRemoveRows(parent, changed.size(), nc-1);
      c.changed = changed;
      endRemoveRows();
    } else if (changed.size() > nc) {
      beginInsertRows(parent, nc, changed.size()-1);
      c.changed = changed;
      endInsertRows();
    }
  }
  
  emitDataChanged(current-1, true);
  emitDataChanged(current, true);
  emitDataChanged(current+1, true);
}

int EditVersionModel::columnCount(const QModelIndex& parent) const
{
  const qint64 internalId = parent.internalId();
  if (internalId >= 0)
    return NCOLUMNS;
  else
    return 1;
}

int EditVersionModel::rowCount(const QModelIndex& parent) const
{
  if (not parent.isValid())
    return mHistory.size();
  const size_t parentRow = parent.row();
  if (parentRow < mHistory.size())
    return mHistory.at(parentRow).changed.size();
  return 0;
}

bool EditVersionModel::hasChildren(const QModelIndex& index) const
{
  return (not index.isValid()) or (index.internalId() < 0);
}

QVariant EditVersionModel::data(const QModelIndex& index, int role) const
{
  const qint64 internalId = index.internalId();
  const int column = index.column();
  if (role == Qt::DisplayRole) {
    if (internalId >= 0) {
      const Change& c = mHistory.at(internalId);
      const ObsData_p obs = c.changed.at(index.row());
      const SensorTime& st = obs->sensorTime();
      switch (column) {
      case COL_TIME:
        return timeutil::shortenedTime(st.time);
      case COL_STATION:
        return st.sensor.stationId;
      case COL_SENSORNR:
        return st.sensor.sensor;
      case COL_LEVEL:
        return st.sensor.level;
      case COL_PARAMID:
        return QString::fromStdString(KvMetaDataBuffer::instance()->findParam(st.sensor.paramId).name());
      case COL_TYPEID:
        return st.sensor.typeId;
      case COL_CORRECTED: {
        const float value = obs->corrected();
        // FIXME copied from ErrorListModel.cc
        if (value == -32767 or value == -32766)
          return QString("-");
        const bool isCodeParam = KvMetaDataBuffer::instance()->isCodeParam(st.sensor.paramId);
        const int nDigits = isCodeParam ? 0 : 1;
        return QString::number(value ,'f', nDigits);
      }
      case COL_FLAGS:
        return Helpers::getFlagText(obs->controlinfo());
      }
    } else if (column == COL_TIME) {
      const timeutil::ptime& t = mHistory.at(index.row()).timestamp;
      return tr("Changed %1").arg(timeutil::shortenedTime(t));
    }
  } else if (role == Qt::ForegroundRole) {
    const size_t version = 1 + ((internalId >= 0) ? internalId : index.row());
    if (version > mDA->currentVersion())
      return Qt::lightGray;
  }
  return QVariant();
}

QModelIndex EditVersionModel::index(int row, int column, const QModelIndex& parent) const
{
  if (not parent.isValid()) {
    if (column == 0  and row < (int)mHistory.size())
      return createIndex(row, column, -1);
  } else {
    const int parentRow = parent.row();
    if (row < (int)mHistory.at(parentRow).changed.size())
      return createIndex(row, column, parentRow);
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

QVariant EditVersionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal and section < NCOLUMNS) {
    if (role == Qt::DisplayRole)
      return qApp->translate("EditVersionModel", headers[section]);
    else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole)
      return qApp->translate("EditVersionModel", tooltips[section]);
  }
  return QVariant();
}
