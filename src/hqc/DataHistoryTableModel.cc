
#include "DataHistoryTableModel.hh"

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
  if (role == Qt::DisplayRole) {
    const kvDataHistoryValues& v = mHistory.at(index.row());
    switch (index.column()) {
    case COL_MTIME:
      return QString::fromStdString(timeutil::to_iso_extended_string(v.modificationtime));
    case COL_CORR: {
      const float value = v.corrected;
      // FIXME this is copied from ErrorListModel
      if (value == -32767 or value == -32766)
        return QString("-");
      const bool isCodeParam = KvMetaDataBuffer::instance()->isCodeParam(mSensorTime.sensor.paramId);
      const int nDigits = isCodeParam ? 0 : 1;
      return QString::number(value ,'f', nDigits);
    }
    case COL_FLAGS:
      return Helpers::getFlagText(v.controlinfo);
    case COL_CHECKS:
      return QString::fromStdString(v.cfailed);
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

  if (eq_SensorTime()(st, mSensorTime))
    return;

  mSensorTime = st;

  dropTask();

  if (not mSensorTime.valid()) {
    beginResetModel();
    mHistory.clear();
    endResetModel();
  } else {
    mTask = new DataHistoryQueryTask(mSensorTime, QueryTask::PRIORITY_AUTOMATIC, this);
    connect(mTask, SIGNAL(completed(const SensorTime&, const kvDataHistoryValues_v&, bool)),
        this, SLOT(onCompleted(const SensorTime&, const kvDataHistoryValues_v&, bool)));
    mKvalobsHandler->postTask(mTask);
  }
}

void DataHistoryTableModel::onCompleted(const SensorTime& st, const kvDataHistoryValues_v& history, bool error)
{
  METLIBS_LOG_SCOPE();
  beginResetModel();
  if (error or not eq_SensorTime()(st, mSensorTime)) {
    mHistory.clear();
  } else {
    mHistory = history;
  }
  METLIBS_LOG_DEBUG(LOGVAL(mHistory.size()));
  endResetModel();
  dropTask();
}

void DataHistoryTableModel::dropTask()
{
  if (not mTask)
    return;

  disconnect(mTask, SIGNAL(completed(const SensorTime&, const kvDataHistoryValues_v&, bool)),
      this, SLOT(onCompleted(const SensorTime&, const kvDataHistoryValues_v&, bool)));
  mKvalobsHandler->dropTask(mTask);
  mTask->deleteLater();
  mTask = 0;
}
