
#ifndef SENSORHEADER_HH
#define SENSORHEADER_HH 1

#include "DataItem.hh"
#include "Sensor.hh"

#include <QtCore/QAbstractTableModel>

class SensorHeader
{
public:
    enum When { NEVER, TOOLTIP, ALWAYS };
    SensorHeader(const Sensor& sensor, When showStation, When showParam, int timeOffset);

    QVariant sensorHeader(DataItemPtr item, Qt::Orientation orientation, int role) const;
    QVariant modelHeader(Qt::Orientation orientation, int role) const;

private:
    QString timeOffset(Qt::Orientation orientation, int role) const;
    QString separator(Qt::Orientation o) const;
    QString displayHeader(Qt::Orientation orientation, const QString& description) const;
    QString stationTooltip() const;

private:
    Sensor mSensor;
    When mShowStation;
    When mShowParam;
    int mTimeOffset;
};

#endif /* SENSORHEADER_HH */
