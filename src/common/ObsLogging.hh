
#ifndef OBSACCESS_OBSLOGGING_HH
#define OBSACCESS_OBSLOGGING_HH 1

#include "Sensor.hh"
#include "util/HqcLogging.hh"
#include "util/boostutil.hh"

std::ostream& operator<<(std::ostream& out, const Sensor& s);
std::ostream& operator<<(std::ostream& out, const Sensor_s& s);
std::ostream& operator<<(std::ostream& out, const SensorTime& st);

#define LOGEBS(obs) " t=" << (obs)->sensorTime().time \
    << " pid=" << (obs)->sensorTime().sensor.paramId                    \
    << " corr=" << (obs)->corrected() << " ci='" << (obs)->controlinfo().flagstring() << "'" \
    << " old_corr=" << (obs)->oldCorrected() << " old_ci='" << (obs)->oldControlinfo().flagstring() << "'" \
    << " tasks=" << (obs)->allTasks()

#define LOGOBS(obs) " t=" << (obs)->sensorTime().time \
    << " pid=" << (obs)->sensorTime().sensor.paramId                    \
    << " corr=" << (obs)->corrected() << " ci='" << (obs)->controlinfo().flagstring() << "'"

#endif // OBSACCESS_OBSLOGGING_HH
