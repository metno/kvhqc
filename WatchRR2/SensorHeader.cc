
#include "SensorHeader.hh"
#include "Helpers.hh"
#include "KvStationBuffer.hh"

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
            if (not header.isEmpty())
                header += "\n";
            header += qApp->translate("SensorHeader", "Parameter %1 %2")
                .arg(Helpers::parameterName(mSensor.paramId))
                .arg(item->description(false));
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
            if (not header.isEmpty())
                header += "\n";
            header += qApp->translate("SensorHeader", "Parameter %1 model value")
                .arg(Helpers::parameterName(mSensor.paramId));
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
        if (not header.isEmpty())
            header += sep;
        header += Helpers::parameterName(mSensor.paramId);
        if (not description.isEmpty())
            header += sep + description;
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
        const kvalobs::kvStation& s = KvStationBuffer::instance()->findStation(mSensor.stationId);
        return qApp->translate("SensorHeader", "Station %1 [%2, %3masl] Level %4")
            .arg(mSensor.stationId)
            .arg(Helpers::stationName(s))
            .arg(s.height(), 0)
            .arg(mSensor.level);
    } catch (std::runtime_error&) {
        return QString("?%1?").arg(mSensor.stationId);
    }
}
