
#include "AnalyseFCC.hh"

#include "FlagChange.hh"
#include "FlagPattern.hh"
#include "Helpers.hh"
#include "Tasks.hh"

#define NDEBUG
#include "w2debug.hh"

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

void analyse(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
{
    LOG_SCOPE();
    using namespace Helpers;
    using namespace detail;

    const std::vector<Sensor> sensors = makeSensors(sensor);

    const boost::gregorian::date_duration step = boost::gregorian::days(1);
    const FlagPattern bad_fcc("fcc=[34]&fhqc=0", FlagPattern::CONTROLINFO);

    for(timeutil::ptime t = time.t0(); t <= time.t1(); t += step) {
        bool have_bad = false;
        std::vector<EditDataPtr> obs(N_COLUMNS);
        for(int i=0; i<detail::N_COLUMNS; ++i) {
            obs[i] = da->findE(SensorTime(sensors[i], timeWithOffset(t, i)));
            if (obs[i] and bad_fcc.matches(obs[i]->controlinfo())) {
                DBG(DBGO1(obs[i]) << DBG1(obs[i]->controlinfo().flagstring()));
                have_bad = true;
            }
        }
        if (have_bad) {
            DBGV(t);
            for(int i=0; i<N_COLUMNS; ++i) {
                if (obs[i])
                    da->editor(obs[i])->addTask(tasks::TASK_FCC_ERROR);
            }
        }
    }
}

void acceptRow(EditAccessPtr da, const Sensor& sensor, const timeutil::ptime& time)
{
    LOG_SCOPE();
    using namespace detail;

    const std::vector<Sensor> sensors = makeSensors(sensor);
    const FlagChange accept_fcc("fhqc=[0234]->fhqc=1"); // FIXME OkCheckTableItem has fmis=0 -- strange; and not fhqc=[34]

    da->pushUpdate();
    for(int i=0; i<N_COLUMNS; ++i) {
        EditDataPtr obs = da->findE(SensorTime(sensors[i], timeWithOffset(time, i)));
        if (obs)
            da->editor(obs)->changeControlinfo(accept_fcc)
                .clearTask(tasks::TASK_FCC_ERROR);
    }
}

} // namespace FCC
