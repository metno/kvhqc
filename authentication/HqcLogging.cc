
#define MILOGGER_CATEGORY "kvhqc.HqcLogging" // only to get rid of warning
#include "HqcLogging.hh"

#include "Sensor.hh"

#include <QtCore/QString>

std::ostream& operator<<(std::ostream& out, const QString& qs)
{
    out << qs.toStdString();
    return out;
}

std::ostream& operator<<(std::ostream& out, const Sensor& s)
{
    out << "(s:" << s.stationId
        << ", p:" << s.paramId
        << ", l:" << s.level
        << ", s:" << s.sensor
        << ", t:" << s.typeId << ')';
    return out;
}

std::ostream& operator<<(std::ostream& out, const SensorTime& st)
{
    out << st.sensor << '@' << st.time;
    return out;
}
