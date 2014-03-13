
#include "ErrorListModel.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/ModelData.hh"
#include "common/KvHelpers.hh"

#include <kvalobs/kvDataOperations.h>

#include <QtCore/QCoreApplication>
#include <QtGui/QColor>
#include <QtGui/QFont>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <algorithm>

#define MILOGGER_CATEGORY "kvhqc.ErrorListModel"
#include "common/ObsLogging.hh"

namespace {

const char* headers[ErrorListModel::NCOLUMNS] = {
  QT_TRANSLATE_NOOP("ErrorList", "Stnr"),
  QT_TRANSLATE_NOOP("ErrorList", "Name"),
  QT_TRANSLATE_NOOP("ErrorList", "WMO"),
  QT_TRANSLATE_NOOP("ErrorList", "Obstime"),
  QT_TRANSLATE_NOOP("ErrorList", "Para"),
  QT_TRANSLATE_NOOP("ErrorList", "Type"),
  QT_TRANSLATE_NOOP("ErrorList", "Orig.d"),
  QT_TRANSLATE_NOOP("ErrorList", "Corr.d"),
  QT_TRANSLATE_NOOP("ErrorList", "mod.v"),
  QT_TRANSLATE_NOOP("ErrorList", "Flags"),
};

const char* tooltips[ErrorListModel::NCOLUMNS] = {
  QT_TRANSLATE_NOOP("ErrorList", "Station number"),
  QT_TRANSLATE_NOOP("ErrorList", "Station name"),
  QT_TRANSLATE_NOOP("ErrorList", "WMO station number"),
  QT_TRANSLATE_NOOP("ErrorList", "Observation time"),
  QT_TRANSLATE_NOOP("ErrorList", "Parameter name"),
  QT_TRANSLATE_NOOP("ErrorList", "Type ID"),
  QT_TRANSLATE_NOOP("ErrorList", "Original value"),
  QT_TRANSLATE_NOOP("ErrorList", "Corrected value"),
  QT_TRANSLATE_NOOP("ErrorList", "Model value"),
  QT_TRANSLATE_NOOP("ErrorList", "Flags"),
};

} // anonymous namespace 

class ErrorListModel::ErrorTreeItem
{
public:
  ErrorTreeItem(const Errors::ErrorInfo& ei, ErrorTreeItem* parent = 0);
  ~ErrorTreeItem();

  ErrorTreeItem* appendChild(ErrorTreeItem *child)
    { mChildren.append(child); return child; }

  ErrorTreeItem* appendChild(const Errors::ErrorInfo& childInfo)
    { return appendChild(new ErrorTreeItem(childInfo, this)); }

  ErrorTreeItem* insertChild(int i, ErrorTreeItem *child)
    { mChildren.insert(i, child); return child; }

  ErrorTreeItem* insertChild(int i, const Errors::ErrorInfo& childInfo)
    { return insertChild(i, new ErrorTreeItem(childInfo, this)); }

  void removeChild(ErrorTreeItem* child)
    { mChildren.removeOne(child); delete child; }

  int childCount() const
    { return mChildren.count(); }

  ErrorTreeItem *child(int row)
    { return mChildren.value(row); }

  EditDataPtr obs() const
    { return mError.obs; }

  const Errors::ErrorInfo& info() const
    { return mError; }

  Errors::ErrorInfo& info()
    { return mError; }

  int row() const;

  ErrorTreeItem *parent()
    { return mParent; }

private:
  Errors::ErrorInfo mError;
  ErrorTreeItem *mParent;
  QList<ErrorTreeItem*> mChildren;
};

ErrorListModel::ErrorTreeItem::ErrorTreeItem(const Errors::ErrorInfo& ei, ErrorTreeItem *parent)
  : mError(ei)
  , mParent(parent)
{
}

ErrorListModel::ErrorTreeItem::~ErrorTreeItem()
{
  qDeleteAll(mChildren);
}

int ErrorListModel::ErrorTreeItem::row() const
{
  if (not mParent)
    return 0;
  return mParent->mChildren.indexOf(const_cast<ErrorTreeItem*>(this));
}

// ========================================================================

ErrorListModel::ErrorListModel(EditAccessPtr eda, ModelAccessPtr mda,
    const Errors::Sensors_t& sensors, const TimeRange& limits, bool errorsForSalen)
  : mDA(eda)
  , mMA(mda)
  , mSensors(sensors)
  , mTimeLimits(limits)
  , mErrorRoot(new ErrorTreeItem(Errors::ErrorInfo(), 0))
  , mErrorsForSalen(errorsForSalen)
  , mShowStation(-1)
{
  mDA->obsDataChanged.connect(boost::bind(&ErrorListModel::onDataChanged, this, _1, _2));

  if (mDA and mTimeLimits.closed()) {
    const Errors::Errors_t errors
        = Errors::fillMemoryStore2(mDA, mSensors, mTimeLimits, mErrorsForSalen);
    buildTree(errors);

    // prefetch model data
    std::vector<SensorTime> sensorTimes;
    BOOST_FOREACH(const Errors::ErrorInfo& ei, errors) {
      sensorTimes.push_back(ei.obs->sensorTime());
    }
    mMA->findMany(sensorTimes);
  }
}

ErrorListModel::~ErrorListModel()
{
  mDA->obsDataChanged.disconnect(boost::bind(&ErrorListModel::onDataChanged, this, _1, _2));
}

ErrorListModel::ErrorTreeItem* ErrorListModel::findParentItem(const QModelIndex& parentIndex) const
{
  if (not parentIndex.isValid())
    return mErrorRoot.get();
  else
    return static_cast<ErrorTreeItem*>(parentIndex.internalPointer());
}

int ErrorListModel::rowCount(const QModelIndex& parent) const
{
  if (parent.column() > 0)
    return 0;
  return findParentItem(parent)->childCount();
}

int ErrorListModel::columnCount(const QModelIndex&) const
{
  return NCOLUMNS;
}

QModelIndex ErrorListModel::index(int row, int column, const QModelIndex &parent) const
{
  if (not hasIndex(row, column, parent))
    return QModelIndex();
  
  ErrorTreeItem *parentItem = findParentItem(parent);
  ErrorTreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex ErrorListModel::index(ErrorTreeItem* item, int column) const
{
  if (not item or item == mErrorRoot.get())
    return QModelIndex();
  return createIndex(item->row(), column, item);
}

QModelIndex ErrorListModel::parent(const QModelIndex &idx) const
{
  if (not idx.isValid())
    return QModelIndex();

  ErrorTreeItem *childItem = static_cast<ErrorTreeItem*>(idx.internalPointer());
  ErrorTreeItem *parentItem = childItem->parent();

  if (parentItem == mErrorRoot.get())
    return QModelIndex();
  
  return index(parentItem);
}

Qt::ItemFlags ErrorListModel::flags(const QModelIndex& /*index*/) const
{
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant ErrorListModel::data(const QModelIndex& index, int role) const
{
  try {
    const EditDataPtr& obs = findObs(index);
    if (not obs)
      return QVariant();
    
    const SensorTime st = obs->sensorTime();
    const int column = index.column();
    if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      if (column <= COL_OBS_TIME)
        return Helpers::stationInfo(st.sensor.stationId) + " "
            + QString::fromStdString(timeutil::to_iso_extended_string(st.time));
      else if (column == COL_OBS_FLAGS)
        return Helpers::getFlagExplanation(obs->controlinfo());
      return QVariant();
    }
    if (role == Qt::DisplayRole) {
      switch (column) {
      case COL_STATION_ID:
        return st.sensor.stationId;
      case COL_STATION_NAME:
        return QString::fromStdString(KvMetaDataBuffer::instance()->findStation(st.sensor.stationId).name());
      case COL_STATION_WMO: {
        const int wmonr = KvMetaDataBuffer::instance()->findStation(st.sensor.stationId).wmonr();
        return (wmonr > 0) ? QVariant(wmonr) : QVariant();
      }
      case COL_OBS_TIME:
        return QString::fromStdString(timeutil::to_iso_extended_string(st.time));
      case COL_OBS_PARAM:
        return QString::fromStdString(KvMetaDataBuffer::instance()->findParam(st.sensor.paramId).name());
      case COL_OBS_TYPEID:
        return st.sensor.typeId;
      case COL_OBS_ORIG:
      case COL_OBS_CORR:
      case COL_OBS_MODEL: {
        float value;
        if (column == COL_OBS_ORIG)
          value = obs->original();
        else if (column == COL_OBS_CORR)
          value = obs->corrected();
        else {
          ModelDataPtr md = mMA->find(st);
          if (not md)
            return QVariant();
          value = md->value();
        }
        if (value == -32767 or value == -32766)
          return QString("-");
        const bool isCodeParam = KvMetaDataBuffer::instance()->isCodeParam(st.sensor.paramId);
        const int nDigits = isCodeParam ? 0 : 1;
        return QString::number(value ,'f', nDigits);
      }
      case COL_OBS_FLAGS:
        return Helpers::getFlagText(obs->controlinfo());
      } // end of switch
    } else if (role == Qt::FontRole) {
      if ((column <= 1 and st.sensor.stationId == mShowStation)
          or (column == COL_OBS_CORR and obs->modifiedCorrected())
          or (column == COL_OBS_FLAGS and obs->modifiedControlinfo()))
      {
        QFont f;
        f.setBold(true);
        return f;
      }
    } else if (role == Qt::ForegroundRole and column == COL_OBS_CORR) {
      const kvalobs::kvControlInfo ci(obs->controlinfo());
      if (ci.flag(kvalobs::flag::fhqc) == 0) { // not hqc touched
        if (ci.qc2dDone())
          return Qt::darkMagenta;
        else if (ci.flag(kvalobs::flag::fnum) >= 6)
          return Qt::red;
      }
    } else if (role == Qt::TextAlignmentRole and (column==COL_OBS_ORIG or column==COL_OBS_CORR or column==COL_OBS_MODEL)) {
      return Qt::AlignRight+Qt::AlignVCenter;
    }
  } catch (std::exception& e) {
    HQC_LOG_WARN("exception: " << e.what());
  } catch (...) {
    HQC_LOG_WARN("exception without message");
  }
  return QVariant();
}

QVariant ErrorListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    if (role == Qt::DisplayRole)
      return qApp->translate("ErrorList", headers[section]);
    else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      const QString tt = qApp->translate("ErrorList", tooltips[section]);
      if (not tt.isEmpty())
        return tt;
    }
  }
  return QVariant();
}

void ErrorListModel::showSameStation(int stationID)
{
  if (mShowStation == stationID)
    return;

  mShowStation = stationID;

  const int nrows = rowCount(QModelIndex()); // FIXME all root rows, or all rows?
  if (nrows > 0)
    Q_EMIT dataChanged(index(0, 0), index(nrows-1, 0));
}

namespace /*anonymous*/ {
struct find_Sensor : public eq_Sensor {
  const Sensor& a;
  find_Sensor(const Sensor& aa) : a(aa) { }
  bool operator()(const Sensor& b) const
    { return eq_Sensor::operator()(a, b); }
};

struct find_ErrorSensorTime : public lt_SensorTime {
  bool operator()(const Errors::ErrorInfo& ei, const SensorTime& st) const
    { return lt_SensorTime::operator()(ei.obs->sensorTime(), st); }
  };
} // anonymous namespace

void ErrorListModel::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr data)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(data->sensorTime()) << LOGVAL(what));

  const SensorTime& st = data->sensorTime();
  if (not mTimeLimits.contains(st.time))
    return;

  if (std::find_if(mSensors.begin(), mSensors.end(), find_Sensor(st.sensor)) == mSensors.end())
    return;

  const QModelIndex idx = findSensorTime(st);

  Q_EMIT beginDataChange();
  if (what == ObsAccess::MODIFIED and idx.isValid()) {
    ErrorTreeItem* item = static_cast<ErrorTreeItem*>(idx.internalPointer());
    Errors::recheck(item->info(), mErrorsForSalen);
    if (not item->info().badInList)
      removeErrorItem(idx);
    else
      updateErrorItem(idx);
  } else if (what == ObsAccess::MODIFIED or what == ObsAccess::CREATED) {
    Errors::ErrorInfo ei(mDA->findE(st));
    Errors::recheck(ei, mErrorsForSalen);
    if (ei.badInList)
      insertErrorItem(ei);
  } else if (what == ObsAccess::DESTROYED) {
    removeErrorItem(idx);
  }
  Q_EMIT endDataChange();
}

void ErrorListModel::updateErrorItem(const QModelIndex& idx)
{
  METLIBS_LOG_SCOPE();
  if (not idx.isValid())
    return;

  const int row = idx.row();
  METLIBS_LOG_DEBUG(LOGVAL(row));
  const QModelIndex p = parent(idx);
  Q_EMIT dataChanged(index(row, COL_OBS_ORIG, p), index(row, COL_OBS_FLAGS, p));
}

void ErrorListModel::removeErrorItem(QModelIndex idx)
{
  METLIBS_LOG_SCOPE();
  if (not idx.isValid())
    return;
  
  ErrorTreeItem* item = static_cast<ErrorTreeItem*>(idx.internalPointer());
  if (item->childCount() > 0) {
    // this item is a group parent; swap with first child and remove child
    ErrorTreeItem* child0 = item->child(0);
    std::swap(child0->info(), item->info());
    updateErrorItem(idx); // this will cause calls to ::data(...)
    item = child0;
    idx = index(0, 0, idx);
    // FIXME as the item data are swapped, the selection is unchanged => no jump to next error
  }
  const int row = item->row();
  METLIBS_LOG_DEBUG("before begin remove" << row);
  beginRemoveRows(parent(idx), row, row);
  METLIBS_LOG_DEBUG("before removeChild");
  item->parent()->removeChild(item);
  METLIBS_LOG_DEBUG("before end remove");
  endRemoveRows();
}

void ErrorListModel::insertErrorItem(Errors::ErrorInfo ei)
{
  METLIBS_LOG_SCOPE();
  const SensorTime st = ei.obs->sensorTime();
  ErrorTreeItem* insertParent = mErrorRoot.get();
  int insertIndex = 0;
  for (; insertIndex < mErrorRoot->childCount(); ++insertIndex) {
    ErrorTreeItem* level0 = mErrorRoot->child(insertIndex);
    SensorTime level0ST = level0->obs()->sensorTime();
    if (lt_Sensor()(st.sensor, level0ST.sensor)) {
      // sensor is before root child's sensor, insert at root level
      break;
    }
    if (eq_Sensor()(st.sensor, level0ST.sensor)) {
      if (st.time < level0ST.time) {
        // need to replace parent
        std::swap(level0->info(), ei);
        updateErrorItem(index(level0)); // this will cause calls to ::data(...)
        insertIndex = 0;
        insertParent = level0;
      } else {
        int insertTime = 0;
        for (; insertTime<level0->childCount(); ++insertTime) {
          ErrorTreeItem* level1 = level0->child(insertTime);
          SensorTime level1ST = level1->obs()->sensorTime();
          if (st.time < level1ST.time)
            break;
        }
        insertIndex = insertTime;
        insertParent = level0;
      }
      break;
    }
  }
  METLIBS_LOG_DEBUG(LOGVAL(insertIndex));
  beginInsertRows(index(insertParent), insertIndex, insertIndex);
  insertParent->insertChild(insertIndex, ei);
  endInsertRows();
}

QModelIndex ErrorListModel::findSensorTime(const SensorTime& st, ErrorTreeItem* item) const
{
  METLIBS_LOG_SCOPE();
  if (item) {
    if (item->obs()) {
      const SensorTime& ist = item->obs()->sensorTime();
      if (eq_SensorTime()(st, ist))
        return index(item);
    }
    for (int i=0; i<item->childCount(); ++i) {
      QModelIndex idx = findSensorTime(st, item->child(i));
      if (idx.isValid())
        return idx;
    }
  }
  return QModelIndex();
}

EditDataPtr ErrorListModel::findObs(const QModelIndex& index) const
{
  if (not index.isValid())
    return EditDataPtr();

  ErrorTreeItem *item = static_cast<ErrorTreeItem*>(index.internalPointer());
  if (not item or item == mErrorRoot.get())
    return EditDataPtr();

  return item->obs();
}

void ErrorListModel::buildTree(const Errors::Errors_t& errors)
{
  ErrorTreeItem* sameItem = 0;
  Sensor sameSensor;
  for (Errors::Errors_t::const_iterator it=errors.begin(); it!=errors.end(); ++it) {
    if (sameItem and eq_Sensor()(sameSensor, it->obs->sensorTime().sensor)) {
      sameItem->appendChild(*it);
    } else {
      sameItem   = mErrorRoot->appendChild(*it);
      sameSensor = sameItem->obs()->sensorTime().sensor;
    }
  }
}
