
#ifndef W2DEBUG_HH
#define W2DEBUG_HH 1

#include "debug.hh"

// for WatchRR2:
#define DBGOO1(obs) " t=" << (obs)->sensorTime().time << " pid=" << (obs)->sensorTime().sensor.paramId \
    << " old_corr=" << (obs)->oldCorrected() << " old_ci='" << (obs)->oldControlinfo().flagstring() << "'" \
    << " tasks=" << (obs)->allTasks()
#define DBGOO(obs)  DBG(DBGON1(obs))

#define DBGO1(obs) " t=" << (obs)->sensorTime().time << " pid=" << (obs)->sensorTime().sensor.paramId \
    << " corr=" << (obs)->corrected() << " ci='" << (obs)->controlinfo().flagstring() << "'"
#define DBGO(obs)  DBG(DBGO1(obs))

#endif // W2DEBUG_HH
