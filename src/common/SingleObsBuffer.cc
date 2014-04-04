
#include "SingleObsBuffer.hh"

#include "util/make_set.hh"

#define MILOGGER_CATEGORY "kvhqc.SingleObsBuffer"
#include "common/ObsLogging.hh"

// ========================================================================

namespace /*anonymous*/ {

} // namespace anonymous

// ========================================================================

SingleObsBuffer::SingleObsBuffer(const SensorTime& st)
  : SimpleBuffer(make_set<Sensor_s>(st.sensor), TimeSpan(st.time, st.time), ObsFilter_p())
{
}

void SingleObsBuffer::onNewData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  if (data.size() == 1) {
    ObsData_p obs = data.front();
    if (match(obs->sensorTime()))
      mObs = obs;
  }
}

void SingleObsBuffer::onUpdateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  onNewData(data);
}

void SingleObsBuffer::onDropData(const SensorTime_v& dropped)
{
  if (dropped.size() == 1 and match(dropped.front()))
    mObs = ObsData_p();
}

bool SingleObsBuffer::match(const SensorTime& st) const
{
  const Sensor& s = *request()->sensors().begin();
  return eq_Sensor()(st.sensor, s)
      and st.time == request()->timeSpan().t0();
}

