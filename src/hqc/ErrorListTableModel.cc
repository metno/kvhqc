
#include "ErrorListTableModel.hh"

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

#define MILOGGER_CATEGORY "kvhqc.ErrorListTableModel"
#include "common/ObsLogging.hh"

namespace {

const char* headers[ErrorListTableModel::NCOLUMNS] = {
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

const char* tooltips[ErrorListTableModel::NCOLUMNS] = {
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

}

ErrorListTableModel::ErrorListTableModel(EditAccessPtr eda, ModelAccessPtr mda,
    const Errors::Sensors_t& sensors, const TimeRange& limits, bool errorsForSalen)
  : mDA(eda)
  , mMA(mda)
  , mSensors(sensors)
  , mTimeLimits(limits)
  , mErrorsForSalen(errorsForSalen)
  , mShowStation(-1)
{
  mDA->obsDataChanged.connect(boost::bind(&ErrorListTableModel::onDataChanged, this, _1, _2));

  if (mDA and mTimeLimits.closed())
    mErrorList = Errors::fillMemoryStore2(mDA, mSensors, mTimeLimits, mErrorsForSalen);

  // prefetch model data
  std::vector<SensorTime> sensorTimes;
  BOOST_FOREACH(const Errors::ErrorInfo& ei, mErrorList) {
    sensorTimes.push_back(ei.obs->sensorTime());
  }
  mMA->findMany(sensorTimes);
}

ErrorListTableModel::~ErrorListTableModel()
{
  mDA->obsDataChanged.disconnect(boost::bind(&ErrorListTableModel::onDataChanged, this, _1, _2));
}

int ErrorListTableModel::rowCount(const QModelIndex&) const
{
  return mErrorList.size();
}

int ErrorListTableModel::columnCount(const QModelIndex&) const
{
  return NCOLUMNS;
}

Qt::ItemFlags ErrorListTableModel::flags(const QModelIndex& /*index*/) const
{
  return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

QVariant ErrorListTableModel::data(const QModelIndex& index, int role) const
{
  //METLIBS_LOG_SCOPE();
  try {
    const Errors::ErrorInfo& ei = mErrorList.at(index.row());
    const EditDataPtr& obs = ei.obs;
    if (not obs)
      return QVariant();
    
    const SensorTime st = obs->sensorTime();
    const int column = index.column();
    if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      if (column <= COL_OBS_TIME)
        return Helpers::stationInfo(st.sensor.stationId) + " "
            + QString::fromStdString(timeutil::to_iso_extended_string(st.time));
      else if (column == COL_OBS_FLAGS)
        return Helpers::getFlagExplanation(ei.obs->controlinfo());
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
        return (ei.badInList != 0) ? Helpers::getFlagText(ei.obs->controlinfo()) : QString("ok");
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

QVariant ErrorListTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

EditDataPtr ErrorListTableModel::mem4Row(int row) const
{
  if (row < 0 or row >= (int)mErrorList.size())
    return EditDataPtr();
  
  return mErrorList.at(row).obs;
}

void ErrorListTableModel::showSameStation(int stationID)
{
  METLIBS_LOG_SCOPE();
  if (mShowStation == stationID)
    return;

  mShowStation = stationID;
  METLIBS_LOG_DEBUG(LOGVAL(mShowStation));
  QModelIndex index1 = createIndex(0, 0);
  QModelIndex index2 = createIndex(mErrorList.size()-1, 0);
  /*emit*/ dataChanged(index1, index2);
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

void ErrorListTableModel::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr data)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(data->sensorTime()) << LOGVAL(what));

  const SensorTime& st = data->sensorTime();
  if (not mTimeLimits.contains(st.time))
    return;

  if (std::find_if(mSensors.begin(), mSensors.end(), find_Sensor(st.sensor)) == mSensors.end())
    return;

  const Errors::Errors_t::iterator itE = std::lower_bound(mErrorList.begin(), mErrorList.end(), st, find_ErrorSensorTime());
  const int position = (itE - mErrorList.begin());
  const bool found = (itE != mErrorList.end() and eq_SensorTime()(itE->obs->sensorTime(), st));
  METLIBS_LOG_DEBUG(LOGVAL(position) << LOGVAL(found) << LOGVAL(mErrorList.size()));

  Errors::ErrorInfo eiInsert;
  bool remove = false;

  if (what == ObsAccess::MODIFIED and found) {
    Errors::recheck(*itE, mErrorsForSalen);
    METLIBS_LOG_DEBUG("re-checked, " << itE->badInList);
    if (not itE->badInList) {
      METLIBS_LOG_DEBUG("was error, now fixed");
      remove = true;
    }
  } else if (what == ObsAccess::MODIFIED or what == ObsAccess::CREATED) {
    Errors::ErrorInfo ei(mDA->findE(st));
    Errors::recheck(ei, mErrorsForSalen);
    METLIBS_LOG_DEBUG("was ok, now error; or created: " << ei.badInList);
    if (ei.badInList) {
      METLIBS_LOG_DEBUG("created error");
      eiInsert = ei;
    }
  } else if (what == ObsAccess::DESTROYED) {
    METLIBS_LOG_DEBUG("error disappeared");
    remove = true;
  }

  if (remove) {
    METLIBS_LOG_DEBUG("remove at " << position << LOGVAL(found));
    if (found) {
      beginRemoveRows(QModelIndex(), position, position);
      mErrorList.erase(itE);
      endRemoveRows();
    }
  } else if (eiInsert.obs) {
    METLIBS_LOG_DEBUG("insert at " << position);
    beginInsertRows(QModelIndex(), position, position);
    mErrorList.insert(itE, eiInsert);
    endInsertRows();
  } else {
    METLIBS_LOG_DEBUG("update at " << position);
    const QModelIndex index0 = createIndex(position, COL_OBS_ORIG);
    const QModelIndex index1 = createIndex(position, COL_OBS_FLAGS);
    /*emit*/ dataChanged(index0, index1);
  }
}

int ErrorListTableModel::findSensorTime(const SensorTime& st)
{
  const Errors::Errors_t::iterator itE = std::lower_bound(mErrorList.begin(), mErrorList.end(), st, find_ErrorSensorTime());
  if (itE != mErrorList.end() and eq_SensorTime()(itE->obs->sensorTime(), st))
    return (itE - mErrorList.begin());
  return -1;
}
