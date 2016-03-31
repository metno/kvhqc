
#include "ErrorListModel.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/ModelData.hh"
#include "common/KvHelpers.hh"
#include "util/stringutil.hh"

#include <kvalobs/kvDataOperations.h>

#include <QtCore/QCoreApplication>
#include <QtGui/QColor>
#include <QtGui/QFont>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <algorithm>

#define MILOGGER_CATEGORY "kvhqc.ErrorListModel"
#include "common/ObsLogging.hh"

//#define ENABLE_HIDE 1
//#define ENABLE_HIGHLIGHT 1

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
  ErrorTreeItem(const Errors::ErrorInfo& ei, ErrorTreeItem_P parent);
  ErrorTreeItem();
  ~ErrorTreeItem();

  ErrorTreeItem_P appendChild(ErrorTreeItem_P child)
    { mChildren.append(child); return child; }

  ErrorTreeItem_P appendChild(const Errors::ErrorInfo& childInfo)
    { return appendChild(new ErrorTreeItem(childInfo, this)); }

  ErrorTreeItem_P insertChild(int i, ErrorTreeItem_P child)
    { mChildren.insert(i, child); return child; }

  ErrorTreeItem_P insertChild(int i, const Errors::ErrorInfo& childInfo)
    { return insertChild(i, new ErrorTreeItem(childInfo, this)); }

  void removeChild(int row)
    { delete mChildren.takeAt(row); }

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

ErrorListModel::ErrorTreeItem::ErrorTreeItem()
  : mParent(0)
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
  , mRootItem(new ErrorTreeItem)
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
  delete mRootItem;
}

ErrorListModel::ErrorTreeItem_P ErrorListModel::itemFromIndex(const QModelIndex& index) const
{
  if (index.isValid()) {
    const ErrorTreeItem_P item = static_cast<ErrorTreeItem_P>(index.internalPointer());
    if (item)
      return item;
  }
  return mRootItem;
}

int ErrorListModel::rowCount(const QModelIndex& parent) const
{
  if (not parent.isValid())
    return mRootItem->childCount();
  else if (parent.column() == 0)
    return itemFromIndex(parent)->childCount();
  else
    return 0;
}

int ErrorListModel::columnCount(const QModelIndex&) const
{
  return NCOLUMNS;
}

QModelIndex ErrorListModel::index(int row, int column, const QModelIndex &parent) const
{
  const ErrorTreeItem_P child = itemFromIndex(parent)->child(row);
  if (child)
    return createIndex(row, column, child);
  else
    return QModelIndex();
}

QModelIndex ErrorListModel::indexFromItem(ErrorTreeItem_P item, int column) const
{
  if (item and item != mRootItem)
    return createIndex(item->row(), column, item);
  else
    return QModelIndex();
}

QModelIndex ErrorListModel::parent(const QModelIndex &index) const
{
  if (index.isValid())
    return indexFromItem(itemFromIndex(index)->parent());
  else
    return QModelIndex();
}

Qt::ItemFlags ErrorListModel::flags(const QModelIndex& index) const
{
  if (index.isValid())
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  else
    return 0;
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
            + Helpers::fromLatin1(timeutil::to_iso_extended_string(st.time));
      else if (column == COL_OBS_FLAGS)
        return Helpers::getFlagExplanation(obs->controlinfo());
      return QVariant();
    }
    if (role == Qt::DisplayRole) {
      switch (column) {
      case COL_STATION_ID:
        return st.sensor.stationId;
      case COL_STATION_NAME:
        return Helpers::fromUtf8(KvMetaDataBuffer::instance()->findStation(st.sensor.stationId).name());
      case COL_STATION_WMO: {
        const int wmonr = KvMetaDataBuffer::instance()->findStation(st.sensor.stationId).wmonr();
        return (wmonr > 0) ? QVariant(wmonr) : QVariant();
      }
      case COL_OBS_TIME:
        return timeutil::to_iso_extended_qstring(st.time);
      case COL_OBS_PARAM:
        return Helpers::fromUtf8(KvMetaDataBuffer::instance()->findParam(st.sensor.paramId).name());
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
    } else if (role == Qt::ForegroundRole) {
      const kvalobs::kvControlInfo ci(obs->controlinfo());
      if (ci.flag(kvalobs::flag::fhqc) == 0) { // not hqc touched
        if (column == COL_OBS_CORR) {
          if (ci.qc2dDone())
            return Qt::darkMagenta;
          else if (ci.flag(kvalobs::flag::fnum) >= 6)
            return Qt::red;
        }
#ifndef ENABLE_HIDE
      } else { // hqc touched
        return Qt::darkGreen;
#endif
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
#ifdef ENABLE_HIGHLIGHT
  if (mHighlightedStation == stationID)
    return;

  mHighlightedStation = stationID;

  if (not mBlockHighlighting) {
    const int nrows = rowCount(); // FIXME all root rows, or all rows?
    if (nrows > 0)
      Q_EMIT dataChanged(index(0, COL_STATION_ID), index(nrows-1, COL_STATION_NAME));
  }
#endif // ENABLE_HIGHLIGHT
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
  METLIBS_LOG_SCOPE(LOGVAL(data->sensorTime()) << LOGVAL(what));

  const SensorTime& st = data->sensorTime();
  if (not mTimeLimits.contains(st.time))
    return;

  if (std::find_if(mSensors.begin(), mSensors.end(), find_Sensor(st.sensor)) == mSensors.end())
    return;

  const QModelIndex index = findSensorTime(st);

  Q_EMIT beginDataChange();
#ifdef ENABLE_HIGHLIGHT
  int highlighted = mHighlightedStation;
  mBlockHighlighting = true;
#endif // ENABLE_HIGHLIGHT

#ifdef ENABLE_HIDE
  if (what == ObsAccess::MODIFIED and index.isValid()) {
    ErrorTreeItem_P item = itemFromIndex(index);
    Errors::recheck(item->info(), mErrorsForSalen);
    if (not item->info().badInList)
      removeErrorItem(item);
    else
      updateErrorItem(item);
  } else if (what == ObsAccess::MODIFIED or what == ObsAccess::CREATED) {
    Errors::ErrorInfo ei(mDA->findE(st));
    Errors::recheck(ei, mErrorsForSalen);
    if (ei.badInList)
      insertErrorItem(ei);
  } else if (what == ObsAccess::DESTROYED) {
    removeErrorItem(itemFromIndex(index));
  }
#else
  if (what == ObsAccess::MODIFIED and index.isValid()) {
    ErrorTreeItem_P item = static_cast<ErrorTreeItem_P>(index.internalPointer());
    Errors::recheck(item->info(), mErrorsForSalen);
    updateErrorItem(item);
  } else if (what == ObsAccess::DESTROYED) {
    removeErrorItem(itemFromIndex(index));
  }
#endif

  Q_EMIT endDataChange();
#ifdef ENABLE_HIGHLIGHT
  mBlockHighlighting = false;
  std::swap(highlighted, mHighlightedStation);
  highlightStation(highlighted);
#endif // ENABLE_HIGHLIGHT
}

void ErrorListModel::updateErrorItem(ErrorTreeItem_P item)
{
  METLIBS_LOG_SCOPE();
  if (item)
    Q_EMIT dataChanged(indexFromItem(item, COL_OBS_ORIG), indexFromItem(item, COL_OBS_FLAGS));
}

void ErrorListModel::removeErrorItem(ErrorTreeItem_P item)
{
  METLIBS_LOG_SCOPE();
  if (not item or item == mRootItem)
    return;

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
  beginRemoveRows(indexFromItem(parent), row, row);
  parent->removeChild(row);
  endRemoveRows();
}

void ErrorListModel::insertErrorItem(Errors::ErrorInfo ei)
{
  METLIBS_LOG_SCOPE();
  const SensorTime& st = ei.obs->sensorTime();
  ErrorTreeItem_P parent = mRootItem;
  int position = 0;
  for (; position < mRootItem->childCount(); ++position) {
    ErrorTreeItem_P level0 = mRootItem->child(position);
    const SensorTime& level0ST = level0->obs()->sensorTime();
    if (lt_Sensor()(st.sensor, level0ST.sensor)) {
      // sensor is before root child's sensor, insert at root level
      break;
    }
    if (eq_Sensor()(st.sensor, level0ST.sensor)) {
      position = 0;
      parent = level0;
      if (st.time < level0ST.time) {
        // need to replace parent
        std::swap(level0->info(), ei);
        updateErrorItem(level0); // this will cause calls to ::data(...)
      } else {
        for (; position<level0->childCount(); ++position) {
          ErrorTreeItem_P level1 = level0->child(position);
          const SensorTime& level1ST = level1->obs()->sensorTime();
          if (st.time < level1ST.time)
            break;
        }
      }
      break;
    }
  }
  METLIBS_LOG_DEBUG(LOGVAL(position));
  beginInsertRows(indexFromItem(parent), position, position);
  parent->insertChild(position, ei);
  endInsertRows();
}

QModelIndex ErrorListModel::findSensorTime(const SensorTime& st, ErrorTreeItem_P item) const
{
  METLIBS_LOG_SCOPE();
  // TODO error list is sorted, should use binary search
  if (item) {
    if (item != mRootItem and item->obs()) {
      if (eq_SensorTime()(st, item->obs()->sensorTime()))
        return indexFromItem(item);
    }
    for (int i=0; i<item->childCount(); ++i) {
      const QModelIndex index = findSensorTime(st, item->child(i));
      if (index.isValid())
        return index;
    }
  }
  return QModelIndex();
}

EditDataPtr ErrorListModel::findObs(const QModelIndex& index) const
{
  const ErrorTreeItem_P item = itemFromIndex(index);
  if (item and item != mRootItem)
    return item->obs();
  else
    return EditDataPtr();
}

void ErrorListModel::buildTree(const Errors::Errors_t& errors)
{
  METLIBS_LOG_SCOPE();
  // FIXME this and remove/insert rely on "errors" being sorted by obs()->sensorTime()
  ErrorTreeItem_P sameItem = 0;
  Sensor sameSensor;
  for (Errors::Errors_t::const_iterator it=errors.begin(); it!=errors.end(); ++it) {
    const Sensor& eSensor = it->obs->sensorTime().sensor;
    if (sameItem and eq_Sensor()(sameSensor, eSensor)) {
      sameItem->appendChild(*it);
    } else {
      sameItem   = mRootItem->appendChild(*it);
      sameSensor = eSensor;
    }
  }
  METLIBS_LOG_DEBUG(LOGVAL(rowCount()) << LOGVAL(mRootItem->childCount()));
}
