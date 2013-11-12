
#include "SensorHeader.hh"

#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "util/Helpers.hh"

#include <QtCore/QCoreApplication>

SensorHeader::SensorHeader(const Sensor& sensor, When showStation, When showParam, int timeOffset)
  : mSensor(sensor)
  , mShowStation(mSensor.stationId >= 0 ? showStation : NEVER)
  , mShowParam(mSensor.paramId >= 0 ? showParam : NEVER)
  , mTimeOffset(timeOffset)
{
}

QVariant SensorHeader::sensorHeader(DataItemPtr item, Qt::Orientation orientation, int role) const
{
  const bool tooltip = (role == Qt::ToolTipRole), display = (role == Qt::DisplayRole);
  if (not tooltip and not display)
    return QVariant();

  QString header;
  if (tooltip) {
    if (mShowStation != NEVER) {
      header = stationTooltip() + " "
          + (qApp->translate("SensorHeader", "Sensor %1 Type %2")
              .arg(mSensor.sensor)
              .arg(mSensor.typeId));
    }
    if (mShowParam != NEVER) {
      Helpers::appendText(header, qApp->translate("SensorHeader", "Parameter %1 %2")
          .arg(Helpers::paramName(mSensor.paramId))
          .arg(item->description(false)), "\n");
    }
  } else {
    header = displayHeader(orientation, item ? item->description(true) : "");
  }
  return header + timeOffset(orientation, role);
}

QVariant SensorHeader::modelHeader(Qt::Orientation orientation, int role) const
{
  const bool tooltip = (role == Qt::ToolTipRole), display = (role == Qt::DisplayRole);
  if (not tooltip and not display)
    return QVariant();

  QString header;
  if (tooltip) {
    if (mShowStation != NEVER)
      header = stationTooltip();
    if (mShowParam != NEVER) {
      Helpers::appendText(header, qApp->translate("SensorHeader", "Parameter %1 model value")
          .arg(Helpers::paramName(mSensor.paramId)), "\n");
    }
  } else {
    header = displayHeader(orientation, qApp->translate("SensorHeader", "model"));
  }
  return header + timeOffset(orientation, display);
}

QString SensorHeader::displayHeader(Qt::Orientation orientation, const QString& description) const
{
  QString header;
  if (mShowStation == ALWAYS)
    header = QString::number(mSensor.stationId);
  if (mShowParam == ALWAYS) {
    const QString sep = separator(orientation);
    Helpers::appendText(header, Helpers::paramName(mSensor.paramId), sep);
    Helpers::appendText(header, QString("T%1").arg(mSensor.typeId), sep);
    if (mSensor.sensor != 0 or mSensor.level != 0)
      Helpers::appendText(header, QString("L%1 S%2").arg(mSensor.level).arg(mSensor.sensor), sep);
    Helpers::appendText(header, description, sep);
  }
  return header;
}

QString SensorHeader::timeOffset(Qt::Orientation orientation, int role) const
{
  if (mTimeOffset == 0)
    return "";

  const QString sep = separator(orientation);
  const bool mini = (role == Qt::DisplayRole);
  if (mTimeOffset > 0) {
    return sep + (mini ? qApp->translate("SensorHeader", "+%1h")
        : qApp->translate("SensorHeader", "time offset +%1 hour(s)"))
        .arg(mTimeOffset);
  } else {
    return sep + (mini ? qApp->translate("SensorHeader", "-%1h")
        : qApp->translate("SensorHeader", "time offset -%1 hour(s)"))
        .arg(-mTimeOffset);
  }
}

QString SensorHeader::separator(Qt::Orientation orientation) const
{
  return (orientation == Qt::Horizontal) ? "\n" : " ";
}

QString SensorHeader::stationTooltip() const
{
  try {
    const kvalobs::kvStation& s = KvMetaDataBuffer::instance()->findStation(mSensor.stationId);
    return qApp->translate("SensorHeader", "Station %1 [%2, %3masl] Level %4")
        .arg(mSensor.stationId)
        .arg(Helpers::stationName(s))
        .arg(s.height(), 0)
        .arg(mSensor.level);
  } catch (std::exception& ) {
    return QString("?%1?").arg(mSensor.stationId);
  }
}
