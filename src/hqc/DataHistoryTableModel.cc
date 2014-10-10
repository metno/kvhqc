
#include "DataHistoryTableModel.hh"

#include "common/ColumnFactory.hh"
#include "common/DataHistoryQueryTask.hh"
#include "common/HqcApplication.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"

#include <QtCore/QVariant>
#include <QtSql/QSqlQuery>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

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

DataHistoryTableModel::DataHistoryTableModel(QObject* parent)
  : QAbstractTableModel(parent)
  , mKvalobsHandler(hqcApp->kvalobsHandler())
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
    DataHistoryQueryTask* t = new DataHistoryQueryTask(st, QueryTask::PRIORITY_AUTOMATIC);
    mTask = new QueryTaskHelper(t);
    connect(mTask, SIGNAL(done(QueryTask*)), this, SLOT(onQueryDone(QueryTask*)));
    mTask->post(mKvalobsHandler.get());
  }
}

void DataHistoryTableModel::onQueryDone(QueryTask* task)
{
  METLIBS_LOG_SCOPE();
  beginResetModel();
  mHistory = static_cast<DataHistoryQueryTask*>(task)->history();
  endResetModel();
  dropTask();
}

void DataHistoryTableModel::dropTask()
{
  delete mTask;
  mTask = 0;
}
