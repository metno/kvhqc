
#include "MissingTableModel.hh"

#include "common/FindExtremeValues.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/ModelData.hh"
#include "util/stringutil.hh"

#include <kvalobs/kvDataOperations.h>

#include <QtCore/QCoreApplication>
#include <QtGui/QFont>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

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

MissingTableModel::MissingTableModel(EditAccessPtr eda, const std::vector<SensorTime>& missing)
  : mDA(eda)
  , mMissing(missing)
{
  METLIBS_LOG_SCOPE();
  mDA->obsDataChanged.connect(boost::bind(&MissingTableModel::onDataChanged, this, _1, _2));
}

MissingTableModel::~MissingTableModel()
{
  mDA->obsDataChanged.disconnect(boost::bind(&MissingTableModel::onDataChanged, this, _1, _2));
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
        return Helpers::stationInfo(st.sensor.stationId) + " "
            + timeutil::to_iso_extended_qstring(st.time);
      else if (column == COL_OBS_TYPEID)
        return Helpers::typeInfo(st.sensor.typeId);
    } else if (role == Qt::DisplayRole) {
      switch (column) {
      case COL_STATION_ID:
        return st.sensor.stationId;
      case COL_STATION_NAME:
        return Helpers::fromUtf8(KvMetaDataBuffer::instance()->findStation(st.sensor.stationId).name());
      case COL_OBSTIME:
        return timeutil::to_iso_extended_qstring(st.time);
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

void MissingTableModel::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr data)
{
  METLIBS_LOG_SCOPE();
  // TODO remove data from missing list
}
