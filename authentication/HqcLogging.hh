
#ifndef HQC_LOGGING_HH
#define HQC_LOGGING_HH 1

#include <Qt/qglobal.h>
#include <iosfwd>

QT_BEGIN_NAMESPACE;
class QString;
QT_END_NAMESPACE;

class Sensor;
class SensorTime;

std::ostream& operator<<(std::ostream& out, const QString& qs);
std::ostream& operator<<(std::ostream& out, const Sensor&  s);
std::ostream& operator<<(std::ostream& out, const SensorTime& st);

#define LOGEBS(obs) " t=" << (obs)->sensorTime().time \
    << " pid=" << (obs)->sensorTime().sensor.paramId                    \
    << " corr=" << (obs)->corrected() << " ci='" << (obs)->controlinfo().flagstring() << "'" \
    << " old_corr=" << (obs)->oldCorrected() << " old_ci='" << (obs)->oldControlinfo().flagstring() << "'" \
    << " tasks=" << (obs)->allTasks()

#define LOGOBS(obs) " t=" << (obs)->sensorTime().time \
    << " pid=" << (obs)->sensorTime().sensor.paramId                    \
    << " corr=" << (obs)->corrected() << " ci='" << (obs)->controlinfo().flagstring() << "'"

#include <miLogger/miLogging.h>

#endif // HQC_LOGGING_HH
