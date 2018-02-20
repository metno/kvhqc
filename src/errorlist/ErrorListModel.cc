/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "ErrorListModel.hh"

#include "ErrorFilter.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/ModelData.hh"
#include "common/KvHelpers.hh"

#include <kvalobs/kvDataOperations.h>

#include <QColor>
#include <QCoreApplication>
#include <QFont>

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
  ErrorTreeItem(ObsData_p obs, ErrorTreeItem_P parent);
  ErrorTreeItem();
  ~ErrorTreeItem();

  ErrorTreeItem_P appendChild(ErrorTreeItem_P child)
    { mChildren.append(child); return child; }

  ErrorTreeItem_P appendChild(ObsData_p obs)
    { return appendChild(new ErrorTreeItem(obs, this)); }

  ErrorTreeItem_P insertChild(int i, ErrorTreeItem_P child)
    { mChildren.insert(i, child); return child; }

  ErrorTreeItem_P insertChild(int i, ObsData_p obs)
    { return insertChild(i, new ErrorTreeItem(obs, this)); }

  void removeChild(int row)
    { delete mChildren.takeAt(row); }

  int childCount() const
    { return mChildren.count(); }

  ErrorTreeItem_P child(int row)
    { return mChildren.value(row); }

  ObsData_p obs() const
    { return mObs; }

  ObsData_p& obs()
    { return mObs; }

  int row() const;

  ErrorTreeItem_P parent()
    { return mParent; }

private:
  ObsData_p mObs;
  ErrorTreeItem_P mParent;
  QList<ErrorTreeItem_P> mChildren;
};

ErrorListModel::ErrorTreeItem::ErrorTreeItem(ObsData_p obs, ErrorTreeItem_P parent)
  : mObs(obs)
  , mParent(parent)
{
}

ErrorListModel::ErrorTreeItem::ErrorTreeItem()
  : mParent(0)
{
}

ErrorListModel::ErrorTreeItem::~ErrorTreeItem()
{
  METLIBS_LOG_SCOPE();
  qDeleteAll(mChildren);
}

int ErrorListModel::ErrorTreeItem::row() const
{
  if (not mParent)
    return 0;
  return mParent->mChildren.indexOf(const_cast<ErrorTreeItem_P>(this));
}

// ========================================================================

ErrorListModel::ErrorListModel(ObsAccess_p eda, ModelAccess_p mda)
    : mDA(eda)
    , mModelBuffer(std::make_shared<ModelBuffer>(mda))
    , mRootItem(0)
    , mHighlightedStation(-1)
    , mHideResolved(true)
{
  connect(mModelBuffer.get(), &ModelBuffer::received, this, &ErrorListModel::onModelData);
}

ErrorListModel::~ErrorListModel()
{
  METLIBS_LOG_SCOPE();
  delete mRootItem;
  mRootItem = 0; // seems unnecessary, but shared_ptr + signals can cause callbacks from destructor ... somehow
}

void ErrorListModel::search(const Sensor_v& sensors, const TimeSpan& limits, bool errorsForSalen)
{
  mModelBuffer->clear();

  beginResetModel();
  delete mRootItem;
  mRootItem = new ErrorTreeItem;
  endResetModel();

  if (mDA and limits.closed()) {
    ErrorFilter_p filter = std::make_shared<ErrorFilter>(errorsForSalen);
    mObsBuffer = std::make_shared<TimeBuffer>(Sensor_s(sensors.begin(), sensors.end()), limits, filter);

    TimeBuffer* b = mObsBuffer.get();
    connect(b, &TimeBuffer::bufferCompleted, this, &ErrorListModel::onFetchComplete);
    connect(b, &TimeBuffer::newDataEnd, this, &ErrorListModel::onFetchDataEnd);
    connect(b, &TimeBuffer::updateDataEnd, this, &ErrorListModel::onUpdateDataEnd);
    connect(b, &TimeBuffer::dropDataEnd, this, &ErrorListModel::onDropDataEnd);

    Q_EMIT fetchingData(true);
    mObsBuffer->postRequest(mDA);

    // TODO prefetch model data
  }
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
  if (not mRootItem)
    return 0;
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
    const ObsData_p& obs = findObs(index);
    if (not obs)
      return QVariant();
    
    const SensorTime& st = obs->sensorTime();
    const int column = index.column();
    if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      if (column <= COL_OBS_TIME)
        return KvMetaDataBuffer::instance()->stationInfo(st.sensor.stationId) + " "
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
        float value = 17;
        if (column == COL_OBS_ORIG)
          value = obs->original();
        else if (column == COL_OBS_CORR)
          value = obs->corrected();
        else {
          ModelData_p md = mModelBuffer->get(st);
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
          or (obs->isModified() and (column == COL_OBS_CORR or column == COL_OBS_FLAGS)))
      {
        QFont f;
        f.setBold(true);
        return f;
      }
    } else if (role == Qt::ForegroundRole) {
      const kvalobs::kvControlInfo& ci(obs->controlinfo());
      if (ci.flag(kvalobs::flag::fhqc) == 0) { // not hqc touched
        if (column == COL_OBS_CORR) {
          if (ci.qc2dDone())
            return QColor(Qt::darkMagenta);
          else if (ci.flag(kvalobs::flag::fnum) >= 6)
            return QColor(Qt::red);
        }
#ifndef ENABLE_HIDE
      } else { // hqc touched
        return QColor(Qt::darkGreen);
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

  const int nrows = rowCount(); // FIXME all root rows, or all rows?
  if (nrows > 0)
    Q_EMIT dataChanged(index(0, COL_STATION_ID), index(nrows-1, COL_STATION_NAME));
#endif // ENABLE_HIGHLIGHT
}

bool ErrorListModel::isError(ObsData_p obs) const
{
  ErrorFilter_p filter = std::static_pointer_cast<ErrorFilter>(mObsBuffer->request()->filter());
  return filter->accept(obs, false);
}

void ErrorListModel::onDataNew(ObsData_p obs)
{
  if (isError(obs))
    insertErrorItem(obs);
}

void ErrorListModel::onDataUpdate(ObsData_p obs)
{
  const QModelIndex index = findSensorTime(obs->sensorTime());
  const bool error = isError(obs);

  if (index.isValid()) {
    ErrorTreeItem_P item = itemFromIndex(index);
    if (not error and mHideResolved)
      removeErrorItem(item);
    else
      updateErrorItem(item);
  } else if (error) {
    insertErrorItem(obs);
  }
}

void ErrorListModel::onDataDrop(const SensorTime& st)
{
  const QModelIndex index = findSensorTime(st);
  ErrorTreeItem_P item = itemFromIndex(index);
  removeErrorItem(item);
}

void ErrorListModel::updateErrorItem(ErrorTreeItem_P item)
{
  if (item)
    Q_EMIT dataChanged(indexFromItem(item, COL_OBS_ORIG), indexFromItem(item, COL_OBS_FLAGS));
}

void ErrorListModel::removeErrorItem(ErrorTreeItem_P item)
{
  if (not item or item == mRootItem)
    return;

  if (item->childCount() > 0) {
    // this item is a group parent; swap data with 1st child and remove child
    ErrorTreeItem_P child0 = item->child(0);
    std::swap(child0->obs(), item->obs());
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

void ErrorListModel::insertErrorItem(ObsData_p obs)
{
  const SensorTime& st = obs->sensorTime();
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
        std::swap(level0->obs(), obs);
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
  parent->insertChild(position, obs);
  endInsertRows();
}

QModelIndex ErrorListModel::findSensorTime(const SensorTime& st, ErrorTreeItem_P item) const
{
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

ObsData_p ErrorListModel::findObs(const QModelIndex& index) const
{
  const ErrorTreeItem_P item = itemFromIndex(index);
  if (item and item != mRootItem)
    return item->obs();
  else
    return ObsData_p();
}

void ErrorListModel::onFetchComplete(const QString&)
{
  METLIBS_LOG_SCOPE();
  Q_EMIT fetchingData(false);
}

void ErrorListModel::onFetchDataEnd(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE(LOGVAL(data.size()));
  Q_EMIT beginDataChange();
  for (ObsData_pv::const_iterator it = data.begin(); it != data.end(); ++it)
    onDataNew(*it);
  Q_EMIT endDataChange();
}

void ErrorListModel::onUpdateDataEnd(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE(LOGVAL(data.size()));
  Q_EMIT beginDataChange();
  for (ObsData_pv::const_iterator it = data.begin(); it != data.end(); ++it)
    onDataUpdate(*it);
  Q_EMIT endDataChange();
}

void ErrorListModel::onDropDataEnd(const SensorTime_v& dropped)
{
  METLIBS_LOG_SCOPE(LOGVAL(dropped.size()));
  Q_EMIT beginDataChange();
  for (SensorTime_v::const_iterator it = dropped.begin(); it != dropped.end(); ++it)
    onDataDrop(*it);
  Q_EMIT endDataChange();
}

void ErrorListModel::onModelData(const ModelData_pv& mdata)
{
  for (ModelData_pv::const_iterator it = mdata.begin(); it != mdata.end(); ++it) {
    const QModelIndex index = findSensorTime((*it)->sensorTime()); // FIXME this will try to match sensorNr and typeid, too
    if (index.isValid())
      Q_EMIT dataChanged(index, index);
  }
}
