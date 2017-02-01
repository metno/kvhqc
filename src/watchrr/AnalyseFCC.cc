
#include "AnalyseFCC.hh"
#include "TaskUpdate.hh"

#include "common/FlagChange.hh"
#include "common/FlagPattern.hh"
#include "common/KvHelpers.hh"
#include "common/ObsHelpers.hh"
#include "common/Tasks.hh"

#define MILOGGER_CATEGORY "kvhqc.AnalyseFCC"
#include "common/ObsLogging.hh"

namespace FCC {

namespace /*FCC::*/detail {
extern const int N_COLUMNS = 11;
extern const int pars[N_COLUMNS] = {
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_V4, kvalobs::PARAMID_V5, kvalobs::PARAMID_V6,
  kvalobs::PARAMID_RR_24, kvalobs::PARAMID_SA
};
extern const int timeOffsets[N_COLUMNS] = {
  -18, -18, -18,
  -12, -12, -12,
  0, 0, 0,
  0, 0
};

std::vector<Sensor> makeSensors(const Sensor& sensor)
{
  std::vector<Sensor> sensors(N_COLUMNS, sensor);
  for(int i=0; i<N_COLUMNS; ++i)
    sensors[i].paramId = pars[i];
  return sensors;
}

timeutil::ptime timeWithOffset(const timeutil::ptime& t, int column)
{
  return t + boost::posix_time::hours(timeOffsets[column]);
}

} // namespace FCC::detail

void analyse(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time)
{
  METLIBS_LOG_SCOPE();
  using namespace Helpers;
  using namespace detail;

  const std::vector<Sensor> sensors = makeSensors(sensor);

  const boost::gregorian::date_duration step = boost::gregorian::days(1);
  const FlagPattern bad_fcc("fcc=[34]&fhqc=0", FlagPattern::CONTROLINFO);

  ObsUpdate_pv updates;
  for(timeutil::ptime t = time.t0(); t <= time.t1(); t += step) {
    bool have_bad = false;
    std::vector<ObsData_p> obs(N_COLUMNS);
    for(int i=0; i<detail::N_COLUMNS; ++i) {
      obs[i] = da->findE(SensorTime(sensors[i], timeWithOffset(t, i)));
      if (obs[i] and bad_fcc.matches(obs[i]->controlinfo())) {
        METLIBS_LOG_DEBUG(LOGOBS(obs[i]) << LOGVAL(obs[i]->controlinfo().flagstring()));
        have_bad = true;
      }
    }
    if (have_bad) {
      METLIBS_LOG_DEBUG(LOGVAL(t));
      for(int i=0; i<N_COLUMNS; ++i) {
        if (obs[i]) {
          TaskUpdate_p up = std::static_pointer_cast<TaskUpdate>(da->createUpdate(obs[i]));
          up->addTask(tasks::TASK_FCC_ERROR);
          updates.push_back(up);
        }
      }
    }
  }
  if (not updates.empty())
    da->storeUpdates(updates);
}

void acceptRow(TaskAccess_p da, const Sensor& sensor, const timeutil::ptime& time)
{
  METLIBS_LOG_SCOPE();
  using namespace detail;

  const std::vector<Sensor> sensors = makeSensors(sensor);
  const FlagChange accept_fcc("fhqc=[0234]->fhqc=1"); // FIXME OkCheckTableItem has fmis=0 -- strange; and not fhqc=[34]

  ObsUpdate_pv updates;
  for(int i=0; i<N_COLUMNS; ++i) {
    if (ObsData_p obs = da->findE(SensorTime(sensors[i], timeWithOffset(time, i)))) {
      TaskUpdate_p up = std::static_pointer_cast<TaskUpdate>(da->createUpdate(obs));
      Helpers::changeControlinfo(up, accept_fcc);
      up->clearTask(tasks::TASK_FCC_ERROR);
      updates.push_back(up);
    }
  }
  if (not updates.empty()) {
    da->newVersion();
    da->storeUpdates(updates);
  }
}

} // namespace FCC
