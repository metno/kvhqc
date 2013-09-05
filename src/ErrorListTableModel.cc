
#include "ErrorListTableModel.hh"

#include "hqc_utilities.hh"
#include "KvMetaDataBuffer.hh"
#include "ModelData.hh"
#include "Helpers.hh"

#include <kvalobs/kvDataOperations.h>

#include <QtCore/QCoreApplication>
#include <QtGui/QColor>
#include <QtGui/QFont>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.ErrorListTableModel"
#include "HqcLogging.hh"

namespace {

const char* headers[ErrorListTableModel::NCOLUMNS] = {
    QT_TRANSLATE_NOOP("ErrorList", "Stnr"),
    QT_TRANSLATE_NOOP("ErrorList", "Name"),
    QT_TRANSLATE_NOOP("ErrorList", "WMO"),
    QT_TRANSLATE_NOOP("ErrorList", "Mt"),
    QT_TRANSLATE_NOOP("ErrorList", "Dy"),
    QT_TRANSLATE_NOOP("ErrorList", "Hr"),
    QT_TRANSLATE_NOOP("ErrorList", "Mn"),
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
    QT_TRANSLATE_NOOP("ErrorList", "Obs month"),
    QT_TRANSLATE_NOOP("ErrorList", "Obs day"),
    QT_TRANSLATE_NOOP("ErrorList", "Obs hour"),
    QT_TRANSLATE_NOOP("ErrorList", "Obs minute"),
    QT_TRANSLATE_NOOP("ErrorList", "Parameter name"),
    QT_TRANSLATE_NOOP("ErrorList", "Type ID"),
    QT_TRANSLATE_NOOP("ErrorList", "Original value"),
    QT_TRANSLATE_NOOP("ErrorList", "Corrected value"),
    QT_TRANSLATE_NOOP("ErrorList", "Model value"),
    QT_TRANSLATE_NOOP("ErrorList", "Flags"),
};

}

ErrorListTableModel::ErrorListTableModel(EditAccessPtr eda, ModelAccessPtr mda, const Errors::Errors_t& errorList, bool errorsForSalen)
    : mDA(eda)
    , mMA(mda)
    , mErrorList(errorList)
    , mErrorsForSalen(errorsForSalen)
    , mShowStation(-1)
{
    mDA->obsDataChanged.connect(boost::bind(&ErrorListTableModel::onDataChanged, this, _1, _2));

    // prefetch model data
    std::vector<SensorTime> sensorTimes;
    BOOST_FOREACH(const Errors::ErrorInfo& ei, errorList) {
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

static QString twoDigits(int number)
{
    return QString("%1").arg(number, 2, 10, QLatin1Char('0'));
}

QVariant ErrorListTableModel::data(const QModelIndex& index, int role) const
{
  METLIBS_LOG_SCOPE();
  try {
    const Errors::ErrorInfo& ei = mErrorList.at(index.row());
    const EditDataPtr& obs = ei.obs;
    if (not obs)
      return QVariant();
    
    const SensorTime st = obs->sensorTime();
    const int column = index.column();
    if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      if (column <= COL_OBS_MINUTE)
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
      case COL_OBS_MONTH:
        return twoDigits(st.time.date().month());
      case COL_OBS_DAY:
        return twoDigits(st.time.date().day());
      case COL_OBS_HOUR:
        return twoDigits(st.time.time_of_day().hours());
      case COL_OBS_MINUTE:
        return twoDigits(st.time.time_of_day().minutes());
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
    } else if (role == Qt::BackgroundRole) {
      if (not mErrorsForSalen) {
        if (ei.badInList == Errors::ErrorInfo::BAD_IN_ERRORLIST2012)
          return QColor(0xFF, 0xFF, 0xC0); // yellow
        else if (ei.badInList == Errors::ErrorInfo::BAD_IN_ERRORLIST2013)
          return QColor(0xFF, 0xE0, 0xE0); // rose
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

void ErrorListTableModel::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr data)
{
    METLIBS_LOG_SCOPE();
    METLIBS_LOG_DEBUG(LOGVAL(data->sensorTime()) << LOGVAL(what));
    if (what == ObsAccess::CREATED)
        return; // ignore for now

    for(unsigned int row=0; row<mErrorList.size(); ++row) {
        Errors::ErrorInfo& ei = mErrorList[row];
        if (eq_SensorTime()(data->sensorTime(), ei.obs->sensorTime())) {
            if (what == ObsAccess::MODIFIED) {
                Errors::recheck(ei, mErrorsForSalen);
                const QModelIndex index0 = createIndex(row, COL_OBS_ORIG);
                const QModelIndex index1 = createIndex(row, COL_OBS_FLAGS);
                /*emit*/ dataChanged(index0, index1);
            } else if (what == ObsAccess::DESTROYED) {
                beginRemoveRows(QModelIndex(), row, row);
                mErrorList.erase(mErrorList.begin() + row);
                row -= 1;
                endRemoveRows();
            }
        }
    }
}
