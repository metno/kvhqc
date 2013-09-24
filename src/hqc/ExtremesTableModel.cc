
#include "ExtremesTableModel.hh"

#include "common/FindExtremeValues.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/ModelData.hh"

#include <kvalobs/kvDataOperations.h>

#include <QtCore/QCoreApplication>
#include <QtGui/QFont>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.ExtremesTableModel"
#include "common/ObsLogging.hh"

namespace {

const char* headers[ExtremesTableModel::NCOLUMNS] = {
  QT_TRANSLATE_NOOP("Extremes", "Stnr"),
  QT_TRANSLATE_NOOP("Extremes", "Name"),
  QT_TRANSLATE_NOOP("Extremes", "Time"),
  QT_TRANSLATE_NOOP("Extremes", "Para"),
  QT_TRANSLATE_NOOP("Extremes", "Type"),
  QT_TRANSLATE_NOOP("Extremes", "Orig.d"),
  QT_TRANSLATE_NOOP("Extremes", "Corr.d"),
  QT_TRANSLATE_NOOP("Extremes", "Flags"),
};

const char* tooltips[ExtremesTableModel::NCOLUMNS] = {
  QT_TRANSLATE_NOOP("Extremes", "Station number"),
  QT_TRANSLATE_NOOP("Extremes", "Station name"),
  QT_TRANSLATE_NOOP("Extremes", "Obstime"),
  QT_TRANSLATE_NOOP("Extremes", "Parameter name"),
  QT_TRANSLATE_NOOP("Extremes", "Type ID"),
  QT_TRANSLATE_NOOP("Extremes", "Original value"),
  QT_TRANSLATE_NOOP("Extremes", "Corrected value"),
  QT_TRANSLATE_NOOP("Extremes", "Flags"),
};

}

ExtremesTableModel::ExtremesTableModel(EditAccessPtr eda, const std::vector<SensorTime>& extremes)
  : mDA(eda)
{
  METLIBS_LOG_SCOPE();
  mDA->obsDataChanged.connect(boost::bind(&ExtremesTableModel::onDataChanged, this, _1, _2));
  BOOST_FOREACH(const SensorTime& st, extremes) {
    EditDataPtr obs = mDA->findE(st);
    if (obs) {
      mExtremes.push_back(obs);
    } else {
      HQC_LOG_ERROR("could not retrieve extreme value at " << st);
    }
  }
}

ExtremesTableModel::~ExtremesTableModel()
{
  mDA->obsDataChanged.disconnect(boost::bind(&ExtremesTableModel::onDataChanged, this, _1, _2));
}

int ExtremesTableModel::rowCount(const QModelIndex&) const
{
  return mExtremes.size();
}

int ExtremesTableModel::columnCount(const QModelIndex&) const
{
  return NCOLUMNS;
}

Qt::ItemFlags ExtremesTableModel::flags(const QModelIndex& /*index*/) const
{
  return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

QVariant ExtremesTableModel::data(const QModelIndex& index, int role) const
{
  try {
    const EditDataPtr& obs = mExtremes.at(index.row());
    if (not obs)
      return QVariant();

    const SensorTime st = obs->sensorTime();
    const int column = index.column();
    if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      if (column <= COL_OBSTIME)
        return Helpers::stationInfo(st.sensor.stationId) + " "
            + QString::fromStdString(timeutil::to_iso_extended_string(st.time));
      if (column == COL_OBS_FLAGS)
        return Helpers::getFlagExplanation(obs->controlinfo());
    } else if (role == Qt::DisplayRole) {
      switch (column) {
      case COL_STATION_ID:
        return st.sensor.stationId;
      case COL_STATION_NAME:
        return QString::fromStdString(KvMetaDataBuffer::instance()->findStation(st.sensor.stationId).name());
      case COL_OBSTIME:
        return QString::fromStdString(timeutil::to_iso_extended_string(st.time));
      case COL_OBS_PARAM:
        return QString::fromStdString(KvMetaDataBuffer::instance()->findParam(st.sensor.paramId).name());
      case COL_OBS_TYPEID:
        return st.sensor.typeId;
      case COL_OBS_ORIG:
      case COL_OBS_CORR: {
        float value;
        if (column == COL_OBS_ORIG)
          value = obs->original();
        else
          value = obs->corrected();
        if (value == -32767 or value == -32766)
          return QString("-");
        const bool isCodeParam = KvMetaDataBuffer::instance()->isCodeParam(st.sensor.paramId);
        const int nDigits = isCodeParam ? 0 : 1;
        return QString::number(value ,'f', nDigits);
      }
      case COL_OBS_FLAGS:
        return Helpers::getFlagText(obs->controlinfo());
      } // end of switch
    } else if (role == Qt::ForegroundRole and column == COL_OBS_CORR) {
      const kvalobs::kvControlInfo ci(obs->controlinfo());
      if (ci.flag(kvalobs::flag::fhqc) == 0) { // not hqc touched
        if (ci.qc2dDone())
          return Qt::darkMagenta;
        else if (ci.flag(kvalobs::flag::fnum) >= 6)
          return Qt::red;
      }
    } else if (role == Qt::TextAlignmentRole and (column==COL_OBS_ORIG or column==COL_OBS_CORR)) {
      return Qt::AlignRight+Qt::AlignVCenter;
    }
  } catch (std::exception& e) {
  }
  return QVariant();
}

QVariant ExtremesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    if (role == Qt::DisplayRole)
      return qApp->translate("Extremes", headers[section]);
    else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      const QString tt = qApp->translate("Extremes", tooltips[section]);
      if (not tt.isEmpty())
        return tt;
    }
  }
  return QVariant();
}

void ExtremesTableModel::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr data)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(data->sensorTime()) << LOGVAL(what));
  if (what == ObsAccess::CREATED)
    return; // ignore for now

  for (size_t row=0; row<mExtremes.size(); ++row) {
    EditDataPtr& obs = mExtremes.at(row);
    if (not obs)
      return;
    if (eq_SensorTime()(data->sensorTime(), obs->sensorTime())) {
      if (what == ObsAccess::MODIFIED) {
        const QModelIndex index0 = createIndex(row, COL_OBS_ORIG);
        const QModelIndex index1 = createIndex(row, COL_OBS_FLAGS);
        /*emit*/ dataChanged(index0, index1);
      } else if (what == ObsAccess::DESTROYED) {
        beginRemoveRows(QModelIndex(), row, row);
        mExtremes.erase(mExtremes.begin() + row);
        row -= 1;
        endRemoveRows();
      }
    }
  }
}
