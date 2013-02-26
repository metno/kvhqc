
#include "AnalyseRR24.hh"
#include "FlagChange.hh"
#include "Helpers.hh"
#include "Tasks.hh"

#include <kvalobs/kvDataOperations.h>
#include <QtCore/QCoreApplication>

#define NDEBUG
#include "w2debug.hh"

namespace { // anonymous
void addRR24Task(EditAccessPtr da, const Sensor& sensor, const timeutil::ptime& time, int task)
{
    EditDataPtr obs = da->findOrCreateE(SensorTime(sensor, time));
    da->editor(obs)->addTask(task);
}

const int ALL_RR24_TASKS =
    ((1<<tasks::TASK_MISSING_OBS)
     | (1<<tasks::TASK_HQC_BEFORE_REDIST)
     | (1<<tasks::TASK_HQC_AUTOMATIC)
     | (1<<tasks::TASK_NO_ACCUMULATION_DAYS)
     | (1<<tasks::TASK_NO_ENDPOINT)
     | (1<<tasks::TASK_MIXED_REDISTRIBUTION)
     | (1<<tasks::TASK_MAYBE_ACCUMULATED)
     | (1<<tasks::TASK_PREVIOUSLY_ACCUMULATION));
} // namespace anonymous

namespace RR24 {

bool analyse(EditAccessPtr da, const Sensor& sensor, TimeRange& time)
{
    LOG_SCOPE();
    using namespace Helpers;

    const boost::gregorian::date_duration step = boost::gregorian::days(1);
    const FlagPattern uncertain_fd("fd=3&fhqc=[04]", FlagPattern::CONTROLINFO);

    // mark all accumulation rows at start as unmodifiable because we
    // cannot tell if we see the complete accumulation
    timeutil::ptime mTS = time.t0(), mTE = time.t1();
    for(; mTS <= mTE; mTS += step) {
        EditDataPtr obs = da->findE(SensorTime(sensor, mTS));
        if (not obs)
            continue;
        if (not is_accumulation(obs))
            break;
        if (is_endpoint(obs)) {
            mTS += step;
            break;
        }
    }

    // mark all accumulation rows at end unmodifiable because we
    // cannot tell if we see the complete accumulation
    for(; mTE >= mTS; mTE -= step) {
        EditDataPtr obs = da->findE(SensorTime(sensor, mTE));
        if (not obs)
            continue;
        if (not is_accumulation(obs) or is_endpoint(obs))
            break;
    }
    DBG(DBG1(mTS) << DBG1(mTE));
    time = TimeRange(mTS, mTE);

    // add tasks for RR24 observations if the have errors
    int last_acc = NO, last_endpoint = NO, have_endpoint = NO;
    for (timeutil::ptime t = mTE; t >= mTS; t -= step) {
        EditDataPtr obs = da->findE(SensorTime(sensor, t));
        if (not obs) {
            obs = da->createE(SensorTime(sensor, t));
            da->editor(obs)->addTask(tasks::TASK_MISSING_OBS);
            last_acc = last_endpoint = have_endpoint = false;
            continue;
        }

        int task = 0;
        int acc = is_accumulation(obs), end = is_endpoint(obs);
        DBG(DBG1(t) << DBG1(obs->controlinfo().flagstring()) << DBG1(acc) << DBG1(end));
        if (acc != NO) {
            DBGL;
            const int f_fhqc = obs->controlinfo().flag(kvalobs::flag::fhqc);
            if (acc == BEFORE_REDIST and f_fhqc != 0 and f_fhqc != 4)
                task = tasks::TASK_HQC_BEFORE_REDIST;
            else if (acc == QC2_REDIST and f_fhqc != 0 and f_fhqc != 4 and f_fhqc != 1)
                task = tasks::TASK_HQC_AUTOMATIC;
            if (end != NO) {
                DBGL;
                if (last_endpoint != NO) {
                    DBGL;
                    addRR24Task(da, sensor, t+step, tasks::TASK_NO_ACCUMULATION_DAYS);
                }
                have_endpoint = last_endpoint = end;
            } else {
                DBGL;
                if (have_endpoint == NO) {
                    DBGL;
                    task = tasks::TASK_NO_ENDPOINT;
                } else if (have_endpoint != acc or last_acc != acc) {
                    DBGL;
                    task = tasks::TASK_MIXED_REDISTRIBUTION;
                }
                last_endpoint = NO;
            }
        } else {
            DBGL;
            if (last_endpoint != NO) {
                DBGL;
                addRR24Task(da, sensor, t+step, tasks::TASK_NO_ENDPOINT);
            }
            last_endpoint = have_endpoint = NO;
            if (uncertain_fd.matches(obs->controlinfo()))
                task = tasks::TASK_MAYBE_ACCUMULATED;
        }
        last_acc = acc;
        if (task != 0)
            da->editor(obs)->addTask(task);
    }
    return true;
}

// ========================================================================

void markPreviousAccumulation(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, bool before)
{
    LOG_SCOPE();
    DBGV(time);
    using namespace Helpers;

    const boost::gregorian::date_duration step = boost::gregorian::days(before ? -1 : 1);
    for(timeutil::ptime t = before ? time.t1() : time.t0(); time.contains(t); t += step) {
        EditDataPtr obs = da->findE(SensorTime(sensor, t));
        if (not obs)
            continue;
        if (is_accumulation(obs) == NO)
            break;
        if (not before) {
            DBGV(t);
            da->editor(obs)->addTask(tasks::TASK_PREVIOUSLY_ACCUMULATION);
        }
        if (is_endpoint(obs) != NO)
            break;
        if (before) {
            DBGV(t);
            da->editor(obs)->addTask(tasks::TASK_PREVIOUSLY_ACCUMULATION);
        }
    }
}

// ========================================================================

void redistribute(EditAccessPtr da, const Sensor& sensor, const timeutil::ptime& t0, const TimeRange& editableTime,
                  const std::vector<float>& newCorr)
{
    LOG_SCOPE();
    using namespace Helpers;
    const boost::gregorian::date_duration step = boost::gregorian::days(1);

    std::vector<EditDataPtr> obs(newCorr.size());
    unsigned int redistRow = 0;
    timeutil::ptime t = t0;
    bool newEndpoint = false;
    for (; redistRow < newCorr.size(); t += step, redistRow += 1) {
        EditDataPtr& o = obs[redistRow] = da->findE(SensorTime(sensor, t));
        if (redistRow == newCorr.size()-1)
            newEndpoint = (not o) or (is_endpoint(o) == NO);
        if (not o)
            o = da->createE(SensorTime(sensor, t));
    }

    const FlagChange fc_miss("fd=9;fhqc=6;fmis=3->fmis=1"),
        fc_end("fd=A;fhqc=6"), fc_dryEnd("fmis=4->fmis=0"), fc_wetEnd("fmis=0->fmis=4");

    da->pushUpdate();
    redistRow = 0;
    t = t0;
    for (; redistRow < newCorr.size(); t += step, redistRow += 1) {
        EditDataEditorPtr editor = da->editor(obs[redistRow]);
        const float newC = newCorr.at(redistRow);
        editor->setCorrected(newC);
        if (redistRow == newCorr.size()-1) {
            DBGL;
            editor->changeControlinfo(fc_end)
                .changeControlinfo(newC == -1 ? fc_dryEnd : fc_wetEnd);
        } else
            editor->changeControlinfo(fc_miss);
        editor->clearTasks(ALL_RR24_TASKS);
        editor->commit();
        DBGO(obs[redistRow]);
    }
    
    // mark all accumulation rows around period with task
    markPreviousAccumulation(da, sensor, TimeRange(editableTime.t0(), t0-step), true);
    if (newEndpoint)
        markPreviousAccumulation(da, sensor, TimeRange(t, editableTime.t1()), false);
}

// ========================================================================

void redistributeInQC2(EditAccessPtr da, const Sensor& sensor,
                       const TimeRange& time, const TimeRange& editableTime)
{
    using namespace Helpers;
    const boost::gregorian::date_duration step = boost::gregorian::days(1);

    const FlagChange fc_miss("fd=)79(->fd=2;fd=9->fd=7;fhqc=4"), fc_end("fd=)8A(->fd=4;fd=A->fd=8;fhqc=4"),
        fc_dryEnd("fmis=4->fmis=0"), fc_wetEnd("fmis=0->fmis=4");

    if (not canRedistributeInQC2(da, sensor, time))
        throw std::runtime_error("cannot redistribute this in QC2");

    da->pushUpdate();
    timeutil::ptime t = time.t0();
    for (; t < time.t1(); t += step) {
        EditDataPtr obs = da->findOrCreateE(SensorTime(sensor, t));
        da->editor(obs)->changeControlinfo(fc_miss).clearTasks(ALL_RR24_TASKS);
    }
    EditDataPtr obs = da->findE(SensorTime(sensor, t));
    const bool newEndpoint = obs and (is_endpoint(obs) == NO);
    if (not obs)
        obs = da->createE(SensorTime(sensor, t));
    da->editor(obs)->changeControlinfo(fc_end)
        .changeControlinfo((obs->corrected() == -1 and obs->original() == -1) ? fc_dryEnd : fc_wetEnd)
        .clearTasks(ALL_RR24_TASKS);
    
    // mark all accumulation rows around period with task
    markPreviousAccumulation(da, sensor, TimeRange(editableTime.t0(), time.t0()-step), true);
    if( newEndpoint )
        markPreviousAccumulation(da, sensor, TimeRange(t+step, editableTime.t1()), false);
}

// ========================================================================

bool canRedistributeInQC2(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
{
    if (time.days() < 1)
        return false;

    const boost::gregorian::date_duration step = boost::gregorian::days(1);
    const FlagPattern ok_miss("fmis=[13]", FlagPattern::CONTROLINFO),
        ok_end("fmis=[024]", FlagPattern::CONTROLINFO);

    int acc_type = -1;
    for (timeutil::ptime t = time.t0(); t < time.t1(); t += step) {
        EditDataPtr obs = da->findE(SensorTime(sensor, t));
        if (not obs)
            continue;
        if (not ok_miss.matches(obs->controlinfo()))
            return false;
        if (acc_type < 0)
            acc_type = Helpers::is_accumulation(obs);
        else if (acc_type != Helpers::is_accumulation(obs))
            return false;
    }
    EditDataPtr obs = da->findE(SensorTime(sensor, time.t1()));
    if (not obs or not ok_end.matches(obs->controlinfo()) or obs->original() < -1.0f)
        return false;
    if (acc_type >= 0 and acc_type != Helpers::is_accumulation(obs))
        return false;
    return true;
}

// ========================================================================

void singles(EditAccessPtr da, const Sensor& sensor, const timeutil::ptime& t0, const TimeRange& editableTime,
             const std::vector<float>& newCorrected, const std::vector<int>& acceptReject)
{
    using namespace Helpers;
    const boost::gregorian::date_duration step = boost::gregorian::days(1);
    const FlagChange fc_clear_fd("fd=1");

    da->pushUpdate();
    bool previousWasSingle = false, previousWasAcc = false;
    unsigned int editRow = 0;
    timeutil::ptime t = t0, tMarkStart = editableTime.t0();
    for (; editRow < newCorrected.size(); t += step, editRow += 1) {
        DBG(DBG1(t) << DBG1(previousWasSingle) << DBG1(previousWasAcc));
        EditDataPtr obs = da->findOrCreateE(SensorTime(sensor, t));
        const int ar = acceptReject.at(editRow);

        if (ar == AR_ACCEPT or ar == AR_REJECT) {
            if (not previousWasSingle) {
                if (is_accumulation(obs))
                    markPreviousAccumulation(da, sensor, TimeRange(tMarkStart, t-step), true);
                if (previousWasAcc)
                    markPreviousAccumulation(da, sensor, TimeRange(tMarkStart, t-step), false);
            }
            previousWasAcc = (is_accumulation(obs) and not is_endpoint(obs));
            previousWasSingle = true;

            EditDataEditorPtr editor = da->editor(obs);
            if (ar == AR_ACCEPT)
                Helpers::correct(editor, newCorrected.at(editRow));
            else
                Helpers::reject(editor);
            editor->changeControlinfo(fc_clear_fd).clearTasks(ALL_RR24_TASKS);
        } else if (previousWasSingle) {
            tMarkStart = t;
            DBGV(tMarkStart);
            previousWasSingle = false;
        }
    }
    if (previousWasSingle) {
        tMarkStart = t;
        DBGV(tMarkStart);
    }

    // mark all accumulation rows around period with task
    if (previousWasAcc)
        markPreviousAccumulation(da, sensor, TimeRange(tMarkStart, editableTime.t1()), false);
}

// ========================================================================

float calculateSum(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
{
    const boost::gregorian::date_duration step = boost::gregorian::days(1);

    float sum = 0;
    for (timeutil::ptime t = time.t0(); t <= time.t1(); t += step) {
        EditDataPtr obs = da->findE(SensorTime(sensor, t));
        if (not obs or Helpers::is_missing(obs))
            continue;
        sum += std::max(0.0f, obs->corrected());
    }
    return sum;
}

// ========================================================================

float calculateOriginalSum(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
{
    if (time.days() < 1)
        return kvalobs::MISSING;

    const boost::gregorian::date_duration step = boost::gregorian::days(1);
    for (timeutil::ptime t = time.t0(); t < time.t1(); t += step) {
        EditDataPtr obs = da->findE(SensorTime(sensor, t));
        if (not obs)
            continue;
        if (not Helpers::is_orig_missing(obs))
            return kvalobs::MISSING;
    }
    EditDataPtr obs = da->findE(SensorTime(sensor, time.t1()));
    if (not obs or Helpers::is_orig_missing(obs))
        return kvalobs::MISSING;
    return std::max(0.0f, obs->original());
}

// ========================================================================

bool canAccept(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
{
    LOG_SCOPE();
    const boost::gregorian::date_duration step = boost::gregorian::days(1);

    const FlagPattern acceptable("fhqc=[01234]", FlagPattern::CONTROLINFO);
    for (timeutil::ptime t = time.t0(); t <= time.t1(); t += step) {
        EditDataPtr obs = da->findE(SensorTime(sensor, t));
        if (not obs)
            continue;
        if (not acceptable.matches(obs->controlinfo()))
            return false;
        if ((obs->allTasks() & ALL_RR24_TASKS) != 0)
            return false;
    }
    return true;
}

// ========================================================================

void accept(EditAccessPtr da, const Sensor& sensor, const TimeRange& time)
{
    LOG_SCOPE();
    const boost::gregorian::date_duration step = boost::gregorian::days(1);

    const FlagChange fc_accept("fhqc=[0234]->fhqc=1");
    da->pushUpdate();
    for (timeutil::ptime t = time.t0(); t <= time.t1(); t += step) {
        EditDataPtr obs = da->findE(SensorTime(sensor, t));
        if (not obs)
            continue;
        da->editor(obs)->changeControlinfo(fc_accept);
    }
}

} // namespace RR24
