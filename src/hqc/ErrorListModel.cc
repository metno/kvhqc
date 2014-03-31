
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
  ErrorTreeItem(const Errors::ErrorInfo& ei, ErrorTreeItem_P parent = 0);
  ~ErrorTreeItem();

  ErrorTreeItem_P appendChild(ErrorTreeItem_P child)
    { mChildren.append(child); return child; }

  ErrorTreeItem_P appendChild(const Errors::ErrorInfo& childInfo)
    { return appendChild(new ErrorTreeItem(childInfo, this)); }

  ErrorTreeItem_P insertChild(int i, ErrorTreeItem_P child)
    { mChildren.insert(i, child); return child; }

  ErrorTreeItem_P insertChild(int i, const Errors::ErrorInfo& childInfo)
    { return insertChild(i, new ErrorTreeItem(childInfo, this)); }

  void removeChild(int child)
    { delete mChildren.value(child); mChildren.removeAt(child); }

  int childCount() const
    { return mChildren.count(); }

  ErrorTreeItem_P child(int row)
    { return mChildren.value(row); }

  EditDataPtr obs() const
    { return mError.obs; }

  const Errors::ErrorInfo& info() const
    { return mError; }

  Errors::ErrorInfo& info()
    { return mError; }

  int row() const;

  ErrorTreeItem_P parent()
    { return mParent; }

private:
  Errors::ErrorInfo mError;
  ErrorTreeItem_P mParent;
  QList<ErrorTreeItem_P> mChildren;
};

ErrorListModel::ErrorTreeItem::ErrorTreeItem(const Errors::ErrorInfo& ei, ErrorTreeItem_P parent)
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
  return mParent->mChildren.indexOf(const_cast<ErrorTreeItem_P>(this));
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
  , mHighlightedStation(-1)
  , mBlockHighlighting(false)
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

ErrorListModel::ErrorTreeItem_P ErrorListModel::itemFromIndex(const QModelIndex& index) const
{
  if (not index.isValid())
    return mErrorRoot.get();
  return static_cast<ErrorTreeItem_P>(index.internalPointer());
}

int ErrorListModel::rowCount(const QModelIndex& parent) const
{
  if (parent.column() > 0)
    return 0;
  return itemFromIndex(parent)->childCount();
}

int ErrorListModel::columnCount(const QModelIndex&) const
{
  return NCOLUMNS;
}

QModelIndex ErrorListModel::index(int row, int column, const QModelIndex &parent) const
{
  if (not hasIndex(row, column, parent))
    return QModelIndex();
  
  ErrorTreeItem_P parentItem = itemFromIndex(parent);
  ErrorTreeItem_P childItem = parentItem->child(row);
  if (not childItem)
    return QModelIndex();
  return createIndex(row, column, childItem);
}

QModelIndex ErrorListModel::itemIndex(ErrorTreeItem_P item, int column) const
{
  if (not item or item == mErrorRoot.get())
    return QModelIndex();
  return createIndex(item->row(), column, item);
}

QModelIndex ErrorListModel::parent(const QModelIndex &idx) const
{
  if (not idx.isValid())
    return QModelIndex();

  ErrorTreeItem_P childItem = itemFromIndex(idx);
  ErrorTreeItem_P parentItem = childItem->parent();

  if (parentItem == mErrorRoot.get())
    return QModelIndex();
  
  return itemIndex(parentItem);
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
      if ((column <= COL_STATION_NAME and st.sensor.stationId == mHighlightedStation)
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

void ErrorListModel::highlightStation(int stationID)
{
  if (mHighlightedStation == stationID)
    return;

  mHighlightedStation = stationID;

  if (not mBlockHighlighting) {
    const int nrows = rowCount(QModelIndex()); // FIXME all root rows, or all rows?
    if (nrows > 0)
      Q_EMIT dataChanged(index(0, COL_STATION_ID), index(nrows-1, COL_STATION_NAME));
  }
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
  int highlighted = mHighlightedStation;
  mBlockHighlighting = true;

  if (what == ObsAccess::MODIFIED and idx.isValid()) {
    ErrorTreeItem_P item = static_cast<ErrorTreeItem_P>(idx.internalPointer());
    Errors::recheck(item->info(), mErrorsForSalen);
    if (not item->info().badInList)
      removeErrorItem(idx);
    else
      updateErrorItem(item);
  } else if (what == ObsAccess::MODIFIED or what == ObsAccess::CREATED) {
    Errors::ErrorInfo ei(mDA->findE(st));
    Errors::recheck(ei, mErrorsForSalen);
    if (ei.badInList)
      insertErrorItem(ei);
  } else if (what == ObsAccess::DESTROYED) {
    removeErrorItem(idx);
  }

  Q_EMIT endDataChange();
  mBlockHighlighting = false;
  std::swap(highlighted, mHighlightedStation);
  highlightStation(highlighted);
}

void ErrorListModel::updateErrorItem(ErrorTreeItem_P item)
{
  METLIBS_LOG_SCOPE();
  if (item)
    Q_EMIT dataChanged(itemIndex(item, COL_OBS_ORIG), itemIndex(item, COL_OBS_FLAGS));
}

void ErrorListModel::removeErrorItem(QModelIndex idx)
{
  METLIBS_LOG_SCOPE();

  ErrorTreeItem_P item = itemFromIndex(idx);
  if (not item or item == mErrorRoot.get()) {
    HQC_LOG_WARN("trying to remove invalid/root index");
    return;
  }

  if (item->childCount() > 0) {
    // this item is a group parent; swap data with 1st child and remove child
    ErrorTreeItem_P child0 = item->child(0);
    std::swap(child0->info(), item->info());
    updateErrorItem(item);
    removeRow(item, 0);
    // FIXME as the item data are swapped, the selection is unchanged => no jump to next error
  } else {
    removeRow(item->parent(), item->row());
  }
}

void ErrorListModel::removeRow(ErrorTreeItem_P parent, int row)
{
  beginRemoveRows(itemIndex(parent), row, row);
  parent->removeChild(row);
  endRemoveRows();
}

void ErrorListModel::insertErrorItem(Errors::ErrorInfo ei)
{
  METLIBS_LOG_SCOPE();
  const SensorTime& st = ei.obs->sensorTime();
  ErrorTreeItem_P insertParent = mErrorRoot.get();
  int insertIndex = 0;
  for (; insertIndex < mErrorRoot->childCount(); ++insertIndex) {
    ErrorTreeItem_P level0 = mErrorRoot->child(insertIndex);
    const SensorTime& level0ST = level0->obs()->sensorTime();
    if (lt_Sensor()(st.sensor, level0ST.sensor)) {
      // sensor is before root child's sensor, insert at root level
      break;
    }
    if (eq_Sensor()(st.sensor, level0ST.sensor)) {
      insertIndex = 0;
      insertParent = level0;
      if (st.time < level0ST.time) {
        // need to replace parent
        std::swap(level0->info(), ei);
        updateErrorItem(level0); // this will cause calls to ::data(...)
      } else {
        for (; insertIndex<level0->childCount(); ++insertIndex) {
          ErrorTreeItem_P level1 = level0->child(insertIndex);
          const SensorTime& level1ST = level1->obs()->sensorTime();
          if (st.time < level1ST.time)
            break;
        }
      }
      break;
    }
  }
  METLIBS_LOG_DEBUG(LOGVAL(insertIndex));
  beginInsertRows(itemIndex(insertParent), insertIndex, insertIndex);
  insertParent->insertChild(insertIndex, ei);
  endInsertRows();
}

QModelIndex ErrorListModel::findSensorTime(const SensorTime& st, ErrorTreeItem_P item) const
{
  METLIBS_LOG_SCOPE();
  if (item) {
    if (item != mErrorRoot.get() and item->obs()) {
      if (eq_SensorTime()(st, item->obs()->sensorTime()))
        return itemIndex(item);
    }
    for (int i=0; i<item->childCount(); ++i) {
      const QModelIndex idx = findSensorTime(st, item->child(i));
      if (idx.isValid())
        return idx;
    }
  }
  return QModelIndex();
}

EditDataPtr ErrorListModel::findObs(const QModelIndex& index) const
{
  const ErrorTreeItem_P item = itemFromIndex(index);
  if (not item or item == mErrorRoot.get())
    return EditDataPtr();

  return item->obs();
}

void ErrorListModel::buildTree(const Errors::Errors_t& errors)
{
  ErrorTreeItem_P sameItem = 0;
  Sensor sameSensor;
  for (Errors::Errors_t::const_iterator it=errors.begin(); it!=errors.end(); ++it) {
    const Sensor& eSensor = it->obs->sensorTime().sensor;
    if (sameItem and eq_Sensor()(sameSensor, eSensor)) {
      sameItem->appendChild(*it);
    } else {
      sameItem   = mErrorRoot->appendChild(*it);
      sameSensor = eSensor;
    }
  }
}
