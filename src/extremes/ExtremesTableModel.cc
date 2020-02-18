
#include "ExtremesTableModel.hh"
#include "ExtremesFilter.hh"

#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"

#include <kvalobs/kvDataOperations.h>

#include <QColor>
#include <QCoreApplication>

#define MILOGGER_CATEGORY "kvhqc.ExtremesTableModel"
#include "common/ObsLogging.hh"

namespace /* anonymous  */ {

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

const int N_EXTREMES = 20;

} // anonymous namespace

// ========================================================================

bool ExtremesTableModel::CorrectedOrdering::compare(ObsData_p a, ObsData_p b) const
{
  const int c = compareCorrected(a->corrected(), b->corrected());
  if (c < 0)
    return true;
  else if (c > 0)
    return false;
  else
    return compare(a->sensorTime(), b->sensorTime());
};

bool ExtremesTableModel::CorrectedOrdering::compare(const SensorTime& a, const SensorTime& b) const
{
  return lt_SensorTime()(a, b);
}

int ExtremesTableModel::CorrectedOrdering::compareCorrected(float a, float b) const
{
  if (a == b)
    return 0;
  else if ((ascending and a<b) or (not ascending and a>b))
    return -1;
  else
    return +1;
}

// ========================================================================

ExtremesTableModel::ExtremesTableModel(EditAccess_p eda)
  : mDA(eda)
  , mCountShown(0)
{
}

ExtremesTableModel::~ExtremesTableModel()
{
}

int ExtremesTableModel::rowCount(const QModelIndex&) const
{
  if (not mBuffer)
    return 0;
  return std::min(mCountShown, (int)mBuffer->data().size());
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
    if (not mBuffer)
      return QVariant();
    const ObsData_p& obs = mBuffer->data().at(index.row());
    if (not obs)
      return QVariant();

    const SensorTime st = obs->sensorTime();
    const int column = index.column();
    if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      if (column <= COL_OBSTIME)
        return KvMetaDataBuffer::instance()->stationInfo(st.sensor.stationId) + " "
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
          return QColor(Qt::darkMagenta);
        else if (ci.flag(kvalobs::flag::fnum) >= 6)
          return QColor(Qt::red);
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

void ExtremesTableModel::search(int paramid, const TimeSpan& time)
{
  METLIBS_LOG_SCOPE(LOGVAL(paramid) << LOGVAL(time));

  ExtremesFilter_p ef(new ExtremesFilter(paramid, 2*N_EXTREMES));
  Sensor_s invalid;
  invalid.insert(Sensor());

  const bool ascending = not ef->isMaximumSearch();
  SortedBuffer::Ordering_p ordering = std::make_shared<CorrectedOrdering>(ascending);
  mBuffer = std::make_shared<SortedBuffer>(ordering, invalid, time, ef);

  SortedBuffer* b = mBuffer.get();
  connect(b, &SortedBuffer::newDataBegin, this, &ExtremesTableModel::onBufferChangeBegin);
  connect(b, &SortedBuffer::updateDataBegin, this, &ExtremesTableModel::onBufferChangeBegin);
  connect(b, &SortedBuffer::dropDataBegin, this, &ExtremesTableModel::onBufferChangeBegin);
  connect(b, &SortedBuffer::newDataEnd, this, &ExtremesTableModel::onBufferChangeEnd);
  connect(b, &SortedBuffer::updateDataEnd, this, &ExtremesTableModel::onBufferChangeEnd);
  connect(b, &SortedBuffer::dropDataEnd, this, &ExtremesTableModel::onBufferChangeEnd);

  mBuffer->postRequest(mDA);
}

void ExtremesTableModel::onBufferChangeBegin()
{
  beginResetModel();
}

void ExtremesTableModel::onBufferChangeEnd()
{
  updateCountShown();
  endResetModel();
}

void ExtremesTableModel::updateCountShown()
{
  METLIBS_LOG_SCOPE();
  const int buffered = (int)mBuffer->data().size();
#if 0
  int different = 0;
  mCountShown = 0;
  while (mCountShown < buffered and different < N_EXTREMES) {
    const float value = mBuffer->data().at(mCountShown)->corrected();
    different += 1;
    mCountShown += 1;
    while (mCountShown < buffered
        and value == mBuffer->data().at(mCountShown)->corrected())
      mCountShown += 1;
  }
  METLIBS_LOG_DEBUG(LOGVAL(mCountShown) << LOGVAL(different));
#else
  mCountShown = buffered;
#endif
}

ObsData_p ExtremesTableModel::getObs(int row) const
{
  if (not mBuffer)
    return ObsData_p();
  return mBuffer->data().at(row);
}
