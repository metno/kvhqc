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


#include "DataHistoryTableModel.hh"

#include "common/ColumnFactory.hh"
#include "common/DataHistoryQueryTask.hh"
#include "common/HqcApplication.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"

#include <QSqlQuery>
#include <QVariant>

#define MILOGGER_CATEGORY "kvhqc.DataHistoryTableModel"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {

enum { COL_MTIME, COL_CORR, COL_FLAGS, COL_CHECKS, NCOLUMNS };

const char* headers[NCOLUMNS] = {
  QT_TRANSLATE_NOOP("DataHistoryTableModel", "Mod.time"),
  QT_TRANSLATE_NOOP("DataHistoryTableModel", "Corr"),
  QT_TRANSLATE_NOOP("DataHistoryTableModel", "Flags"),
  QT_TRANSLATE_NOOP("DataHistoryTableModel", "Checks")
};

const char* tooltips[NCOLUMNS] = {
  QT_TRANSLATE_NOOP("DataHistoryTableModel", "Modificationtime"),
  QT_TRANSLATE_NOOP("DataHistoryTableModel", "Corrected value"),
  "",
  ""
};

} // anonymous namespace

DataHistoryTableModel::DataHistoryTableModel(QueryTaskHandler_p handler, QObject* parent)
  : QAbstractTableModel(parent)
  , mHandler(handler)
  , mTask(0)
{
}

DataHistoryTableModel::~DataHistoryTableModel()
{
  dropTask();
}

int DataHistoryTableModel::rowCount(const QModelIndex&) const
{
  return mHistory.size();
}

int DataHistoryTableModel::columnCount(const QModelIndex&) const
{
  return NCOLUMNS;
}

Qt::ItemFlags DataHistoryTableModel::flags(const QModelIndex& /*index*/) const
{
  return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

QVariant DataHistoryTableModel::data(const QModelIndex& index, int role) const
{
  const kvDataHistoryValues& v = history().at(index.row());
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case COL_MTIME:
      return timeutil::shortenedTime(v.modificationtime);
    case COL_CORR: {
      if (mCode2Text)
        return mCode2Text->asText(v.corrected);
      else
        return QVariant(v.corrected);
    }
    case COL_FLAGS:
      return Helpers::getFlagText(v.controlinfo);
    case COL_CHECKS:
      return QString::fromStdString(v.cfailed);
    }
  } else if (role == Qt::ToolTipRole) {
    switch (index.column()) {
    case COL_MTIME:
      return QString::fromStdString(timeutil::to_iso_extended_string(v.modificationtime));
    case COL_CORR: {
      if (mCode2Text)
        return mCode2Text->asTip(v.corrected);
      break;
    }
    case COL_FLAGS:
      return Helpers::getFlagExplanation(v.controlinfo);
    }
  }
  return QVariant();
}

QVariant DataHistoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    if (role == Qt::DisplayRole)
      return tr(headers[section]);
    else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      const QString tt = tr(tooltips[section]);
      if (not tt.isEmpty())
        return tt;
    }
  }
  return QVariant();
}

void DataHistoryTableModel::showHistory(const SensorTime& st)
{
  METLIBS_LOG_SCOPE();

  dropTask();
  if (not st.valid()) {
    beginResetModel();
    mCode2Text = Code2TextCPtr();
    mHistory.clear();
    endResetModel();
  } else {
    mCode2Text = ColumnFactory::codesForParam(st.sensor.paramId);
    mTask = new DataHistoryQueryTask(st, QueryTask::PRIORITY_AUTOMATIC);
    connect(mTask, SIGNAL(taskDone(const QString&)), this, SLOT(onTaskDone(const QString&)));
    mHandler->postTask(mTask);
  }
}

void DataHistoryTableModel::onTaskDone(const QString&)
{
  METLIBS_LOG_SCOPE(LOGVAL(mTask));
  beginResetModel();
  if (mTask)
    mHistory = mTask->history();
  else
    mHistory.clear();
  METLIBS_LOG_DEBUG(LOGVAL(mHistory.size()));
  endResetModel();
  dropTask();
}

void DataHistoryTableModel::dropTask()
{
  METLIBS_LOG_SCOPE();
  if (mTask) {
    mHandler->dropTask(mTask);
    mTask->deleteWhenDone();
    disconnect(mTask, SIGNAL(taskDone(const QString&)), this, SLOT(onTaskDone(const QString&)));
    mTask = 0;
  }
}
