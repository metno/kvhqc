
#include "EditVersionModel.hh"

#include "common/KvHelpers.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.EditVersionModel"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {

enum EDIT_COLUMNS {
  COL_TIME = 0,
  COL_SENSOR,
  COL_CORRECTED,
  COL_FLAGS,
  NCOLUMNS
};

const char* headers[NCOLUMNS] = {
  QT_TRANSLATE_NOOP("ErrorList", "Time"),
  QT_TRANSLATE_NOOP("ErrorList", "Station"),
  QT_TRANSLATE_NOOP("ErrorList", "Corr."),
  QT_TRANSLATE_NOOP("ErrorList", "Flags")
};

const char* tooltips[NCOLUMNS] = {
  QT_TRANSLATE_NOOP("ErrorList", "Change time / Observation time"),
  QT_TRANSLATE_NOOP("ErrorList", "Station / Type Parameter"),
  QT_TRANSLATE_NOOP("ErrorList", "New Corrected Value"),
  QT_TRANSLATE_NOOP("ErrorList", "New Flags")
};

} // namespace anonymous

EditVersionModel::EditVersionModel(EditAccess_p eda, QObject* parent)
  : QAbstractItemModel(parent)
  , mDA(eda)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(mDA->currentVersion()) << LOGVAL(mDA->highestVersion()));
  //mDA->currentVersionChanged.connect(boost::bind(&EditVersionModel::onCurrentVersionChanged, this, _1, _2));
  //mDA->obsDataChanged       .connect(boost::bind(&EditVersionModel::onDataChanged,           this, _1, _2));
}

EditVersionModel::~EditVersionModel()
{
  //mDA->obsDataChanged       .disconnect(boost::bind(&EditVersionModel::onDataChanged,           this, _1, _2));
  //mDA->currentVersionChanged.disconnect(boost::bind(&EditVersionModel::onCurrentVersionChanged, this, _1, _2));
}

//void EditVersionModel::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
//{
//  METLIBS_LOG_SCOPE();
//  dump();
//}

void EditVersionModel::onCurrentVersionChanged(int current, int highest)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(current) << LOGVAL(highest));
  dump();
}

void EditVersionModel::dump()
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(mDA->currentVersion()) << LOGVAL(mDA->highestVersion()));
  Q_EMIT beginResetModel();
#if 0
  mHistory = ChangeHistory_t();
  for(int v=1; v<=mDA->highestVersion(); ++v) {
    mHistory.push_back(mDA->versionChanges(v));
#if 1
    const std::vector<EditDataPtr>& changes = mHistory.back();
    METLIBS_LOG_DEBUG("changes for version " << v << " from " << mDA->versionTimestamp(v) << ":");
    BOOST_FOREACH(EditDataPtr obs, changes)
        METLIBS_LOG_DEBUG("   " << obs->sensorTime() << " c=" << obs->corrected(v) << " f=" << obs->controlinfo(v).flagstring());
#endif
  }
#endif
  Q_EMIT endResetModel();
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
#if 0
  if (not parent.isValid())
    return mHistory.size();
  const size_t parentRow = parent.row();
  if (parentRow >= mHistory.size())
#endif
    return 0;
  //return mHistory.at(parentRow).size();
}

QVariant EditVersionModel::data(const QModelIndex& index, int role) const
{
  const qint64 internalId = index.internalId();
  const int column = index.column();
#if 0  
  if (role == Qt::DisplayRole) {
    if (internalId >= 0) {
      const int version = internalId+1;
      const EditDataPtr obs = mHistory.at(internalId).at(index.row());
      const SensorTime& st = obs->sensorTime();
      switch (column) {
      case COL_TIME:
        return QString::fromStdString(timeutil::to_iso_extended_string(st.time));
      case COL_SENSOR:
        return QString("%1/%2 %3").arg(st.sensor.stationId)
            .arg(st.sensor.typeId)
            .arg(st.sensor.paramId);
      case COL_CORRECTED:
        return Helpers::formatValue(obs->corrected(version));
      case COL_FLAGS:
        return Helpers::getFlagText(obs->controlinfo(version));
      }
    } else if (column == 0) {      
      const int version = index.row()+1;
      const timeutil::ptime& t = mDA->versionTimestamp(version);
      return tr("Changed %1").arg(QString::fromStdString(timeutil::to_iso_extended_string(t)));
    }
  } else if (role == Qt::ForegroundRole) {
    const int version = 1 + ((internalId >= 0) ? internalId : index.row());
    if (version > mDA->currentVersion())
      return Qt::lightGray;
  }
#endif
  return QVariant();
}

bool EditVersionModel::hasChildren(const QModelIndex& index) const
{
  return (not index.isValid()) or (index.internalId() < 0);
}

QModelIndex EditVersionModel::index(int row, int column, const QModelIndex& parent) const
{
#if 0
  if (not parent.isValid()) {
    if (column == 0  and row < (int)mHistory.size())
      return createIndex(row, column, -1);
  } else {
    const int parentRow = parent.row();
    if (row < (int)mHistory.at(parentRow).size())
      return createIndex(row, column, parentRow);
  }
#endif
  return QModelIndex();
}

QModelIndex EditVersionModel::parent(const QModelIndex& index) const
{
#if 0
  const qint64 internalId = index.internalId();
  if (internalId >= 0)
    return createIndex(internalId, 0, -1);
  else
#endif
    return QModelIndex();
}

QVariant EditVersionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal and section < NCOLUMNS) {
    if (role == Qt::DisplayRole)
      return headers[section];
    else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole)
      return tooltips[section];
  }
  return QVariant();
}
