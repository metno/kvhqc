
#include "MissingTableModel.hh"

#include "MissingObsQuery.hh"

#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"

#include <kvalobs/kvDataOperations.h>

#include <QCoreApplication>

#define MILOGGER_CATEGORY "kvhqc.MissingTableModel"
#include "common/ObsLogging.hh"

namespace {

const char* headers[MissingTableModel::NCOLUMNS] = {
  QT_TRANSLATE_NOOP("Missing", "Stnr"),
  QT_TRANSLATE_NOOP("Missing", "Name"),
  QT_TRANSLATE_NOOP("Missing", "Time"),
  QT_TRANSLATE_NOOP("Missing", "Type"),
};

const char* tooltips[MissingTableModel::NCOLUMNS] = {
  QT_TRANSLATE_NOOP("Missing", "Station number"),
  QT_TRANSLATE_NOOP("Missing", "Station name"),
  QT_TRANSLATE_NOOP("Missing", "Obstime"),
  QT_TRANSLATE_NOOP("Missing", "Type ID"),
};

}

MissingTableModel::MissingTableModel(QueryTaskHandler_p handler)
  : mKvalobsHandler(handler)
  , mTask(0)
{
}

MissingTableModel::~MissingTableModel()
{
}

int MissingTableModel::rowCount(const QModelIndex&) const
{
  return mMissing.size();
}

int MissingTableModel::columnCount(const QModelIndex&) const
{
  return NCOLUMNS;
}

Qt::ItemFlags MissingTableModel::flags(const QModelIndex& /*index*/) const
{
  return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

QVariant MissingTableModel::data(const QModelIndex& index, int role) const
{
  try {
    const SensorTime& st = mMissing.at(index.row());
    const int column = index.column();
    if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      if (column <= COL_OBSTIME)
        return KvMetaDataBuffer::instance()->stationInfo(st.sensor.stationId) + " "
            + QString::fromStdString(timeutil::to_iso_extended_string(st.time));
      else if (column == COL_OBS_TYPEID)
        return KvMetaDataBuffer::instance()->typeInfo(st.sensor.typeId);
    } else if (role == Qt::DisplayRole) {
      switch (column) {
      case COL_STATION_ID:
        return st.sensor.stationId;
      case COL_STATION_NAME:
        return QString::fromStdString(KvMetaDataBuffer::instance()->findStation(st.sensor.stationId).name());
      case COL_OBSTIME:
        return QString::fromStdString(timeutil::to_iso_extended_string(st.time));
      case COL_OBS_TYPEID:
        return st.sensor.typeId;
      } // end of switch
    }
  } catch (std::exception& e) {
  }
  return QVariant();
}

QVariant MissingTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    if (role == Qt::DisplayRole)
      return qApp->translate("Missing", headers[section]);
    else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      const QString tt = qApp->translate("Missing", tooltips[section]);
      if (not tt.isEmpty())
        return tt;
    }
  }
  return QVariant();
}

void MissingTableModel::search(const TimeSpan& time, const hqc::int_s& typeIds)
{
  METLIBS_LOG_SCOPE();

  MissingObsQuery* t = new MissingObsQuery(time, typeIds, QueryTask::PRIORITY_AUTOMATIC);
  METLIBS_LOG_DEBUG(t->querySql("d=postgresql"));

  dropTask();
  mTask = new QueryTaskHelper(t);
  connect(mTask, SIGNAL(done(QueryTask*)), this, SLOT(onQueryDone(QueryTask*)));
  mTask->post(mKvalobsHandler.get());
}

void MissingTableModel::onQueryDone(QueryTask* task)
{
  METLIBS_LOG_SCOPE();
  beginResetModel();
  mMissing = static_cast<MissingObsQuery*>(task)->missing();
  endResetModel();
  dropTask();
}

void MissingTableModel::dropTask()
{
  delete mTask;
  mTask = 0;
}
