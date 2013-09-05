
#ifndef HQC_LOGGING_HH
#define HQC_LOGGING_HH 1

#include "HqcApplication.hh"
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

#define HQC_LOG_WARN(x)                                 \
  do { if (hqcApp) { hqcApp->setReturnCode(2); }        \
    METLIBS_LOG_WARN(x); } while(0)

#define HQC_LOG_ERROR(x)                                \
  do { if (hqcApp) { hqcApp->setReturnCode(2); }        \
    METLIBS_LOG_ERROR(x); } while(0)

#endif // HQC_LOGGING_HH
