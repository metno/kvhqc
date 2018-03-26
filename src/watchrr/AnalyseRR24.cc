/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "AnalyseRR24.hh"
#include "TaskData.hh"
#include "TaskUpdate.hh"

#include "common/AcceptReject.hh"
#include "common/FlagChange.hh"
#include "common/Functors.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/ObsHelpers.hh"
#include "common/Tasks.hh"

#include "util/Helpers.hh"

#include <QCoreApplication>
#include <kvalobs/kvDataOperations.h>

#define MILOGGER_CATEGORY "kvhqc.AnalyseRR24"
#include "common/ObsLogging.hh"

namespace { // anonymous

TaskUpdate_p createU(TaskAccess_p da, const SensorTime& st)
{
  ObsData_p obs = da->findE(st);
  return std::static_pointer_cast<TaskUpdate>(obs ? da->createUpdate(obs) : da->createUpdate(st));
}

void addRR24Task(TaskAccess_p da, const Sensor& sensor, const timeutil::ptime& time, int task)
{
  TaskUpdate_p up = createU(da, SensorTime(sensor, time));
  up->addTask(task);
  da->storeUpdates(ObsUpdate_pv(1, up));
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

bool analyse(TaskAccess_p da, const Sensor& sensor, TimeSpan& time)
{
  METLIBS_LOG_SCOPE();
  using namespace Helpers;

  const boost::gregorian::date_duration step = boost::gregorian::days(1);
  const FlagPattern uncertain_fd("fd=3&fhqc=[04]", FlagPattern::CONTROLINFO);

  // mark all accumulation rows at start as unmodifiable because we
  // cannot tell if we see the complete accumulation
  timeutil::ptime mTS = time.t0(), mTE = time.t1();
  for(; mTS <= mTE; mTS += step) {
    ObsData_p obs = da->findE(SensorTime(sensor, mTS));
    if (not obs)
      continue;
    if (is_accumulation(obs) == NO)
      break;
    if (is_endpoint(obs) != NO) {
      mTS += step;
      break;
    }
  }

  // mark all accumulation rows at end unmodifiable because we
  // cannot tell if we see the complete accumulation
  for(; mTE >= mTS; mTE -= step) {
    ObsData_p obs = da->findE(SensorTime(sensor, mTE));
    if (not obs)
      continue;
    if ((is_accumulation(obs) == NO) or (is_endpoint(obs) != NO))
      break;
  }
  time = TimeSpan(mTS, mTE);

  ObsUpdate_pv updates;

  // add tasks for RR24 observations if the have errors
  int last_acc = NO, last_endpoint = NO, have_endpoint = NO;
  for (timeutil::ptime t = mTE; t >= mTS; t -= step) {
    const SensorTime st(sensor, t);
    ObsData_p obs = da->findE(st);
    if (not obs) {
      TaskUpdate_p up = createU(da, st);
      up->addTask(tasks::TASK_MISSING_OBS);
      updates.push_back(up);
      last_acc = last_endpoint = have_endpoint = false;
      continue;
    }

    int task = 0;
    int acc = is_accumulation(obs), end = is_endpoint(obs);
    METLIBS_LOG_DEBUG(LOGVAL(t) << LOGVAL(obs->controlinfo().flagstring()) << LOGVAL(acc) << LOGVAL(end));
    if (acc != NO) {
      const int f_fhqc = obs->controlinfo().flag(kvalobs::flag::fhqc);
      if (acc == BEFORE_REDIST and f_fhqc != 0 and f_fhqc != 4)
        task = tasks::TASK_HQC_BEFORE_REDIST;
      else if (acc == QC2_REDIST and f_fhqc != 0 and f_fhqc != 4 and f_fhqc != 1)
        task = tasks::TASK_HQC_AUTOMATIC;
      if (end != NO) {
        if (last_endpoint != NO) {
          addRR24Task(da, sensor, t+step, tasks::TASK_NO_ACCUMULATION_DAYS);
        }
        have_endpoint = last_endpoint = end;
      } else {
        if (have_endpoint == NO) {
          task = tasks::TASK_NO_ENDPOINT;
        } else if (have_endpoint != acc or last_acc != acc) {
          task = tasks::TASK_MIXED_REDISTRIBUTION;
        }
        last_endpoint = NO;
      }
    } else {
      if (last_endpoint != NO) {
        addRR24Task(da, sensor, t+step, tasks::TASK_NO_ENDPOINT);
      }
      last_endpoint = have_endpoint = NO;
      if (uncertain_fd.matches(obs->controlinfo()))
        task = tasks::TASK_MAYBE_ACCUMULATED;
    }
    last_acc = acc;
    if (task != 0) {
      TaskUpdate_p up = createU(da, st);
      up->addTask(task);
      updates.push_back(up);
    }
  }
  da->storeUpdates(updates);
  return true;
}

// ========================================================================

void markPreviousAccumulation(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time, bool before)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(time));
  using namespace Helpers;

  ObsUpdate_pv updates;

  const boost::gregorian::date_duration step = boost::gregorian::days(before ? -1 : 1);
  for(timeutil::ptime t = before ? time.t1() : time.t0(); time.contains(t); t += step) {
    ObsData_p obs = da->findE(SensorTime(sensor, t));
    if (not obs)
      continue;
    if (is_accumulation(obs) == NO)
      break;
    TaskUpdate_p up;
    if (not before) {
      METLIBS_LOG_DEBUG(LOGVAL(t));
      up = createU(da, SensorTime(sensor, t));
    }
    if (is_endpoint(obs) != NO)
      break;
    if (before) {
      METLIBS_LOG_DEBUG(LOGVAL(t));
      up = createU(da, SensorTime(sensor, t));
    }
    if (up) {
      up->addTask(tasks::TASK_PREVIOUSLY_ACCUMULATION);
      updates.push_back(up);
    }
  }

  da->storeUpdates(updates);
}

// ========================================================================

void redistribute(TaskAccess_p da, const Sensor& sensor, const timeutil::ptime& t0, const TimeSpan& editableTime,
    const std::vector<float>& newCorr)
{
  METLIBS_LOG_SCOPE();
  using namespace Helpers;
  const boost::gregorian::date_duration step = boost::gregorian::days(1);

  timeutil::ptime t = t0;

  ObsUpdate_pv obsu(newCorr.size());
  for (size_t r=0; r < newCorr.size(); r += 1, t += step)
    obsu[r] = createU(da, SensorTime(sensor, t));

  ObsData_p obs_endpoint = da->findE(SensorTime(sensor, t - step));
  const bool newEndpoint = (not obs_endpoint) or (is_endpoint(obs_endpoint) == NO);

  const FlagChange fc_miss("fd=9;fhqc=6;fmis=3->fmis=1"),
      fc_end("fd=A;fhqc=6"), fc_dryEnd("fmis=4->fmis=0"), fc_wetEnd("fmis=0->fmis=4");

  da->newVersion();
  t = t0;
  for (size_t r=0; r < newCorr.size(); r += 1, t += step) {
    const ObsUpdate_p& u = obsu[r];
    const float newC = newCorr.at(r);
    u->setCorrected(newC);
    if (r == newCorr.size()-1) {
      const float oldC = u->corrected();
      Helpers::changeControlinfo(u, fc_end);
      if (newC == -1 and oldC == -1)
        Helpers::changeControlinfo(u, fc_dryEnd);
      else
        Helpers::changeControlinfo(u, fc_wetEnd);
    } else
      Helpers::changeControlinfo(u, fc_miss);

    TaskUpdate_p tu = std::static_pointer_cast<TaskUpdate>(u);
    tu->clearTasks(ALL_RR24_TASKS);
  }
  da->storeUpdates(obsu);

  // mark all accumulation rows around period with task
  markPreviousAccumulation(da, sensor, TimeSpan(editableTime.t0(), t0-step), true);
  if (newEndpoint)
    markPreviousAccumulation(da, sensor, TimeSpan(t, editableTime.t1()), false);
}

// ========================================================================

void redistributeInQC2(TaskAccess_p da, const Sensor& sensor,
    const TimeSpan& time, const TimeSpan& editableTime)
{
  using namespace Helpers;
  const boost::gregorian::date_duration step = boost::gregorian::days(1);

  const FlagChange fc_miss("fd=)79(->fd=2;fd=9->fd=7;fhqc=4"), fc_end("fd=)8A(->fd=4;fd=A->fd=8;fhqc=4"),
      fc_dryEnd("fmis=4->fmis=0"), fc_wetEnd("fmis=0->fmis=4");

  if (not canRedistributeInQC2(da, sensor, time))
    throw std::runtime_error("cannot redistribute this in QC2");

  da->newVersion();
  ObsUpdate_pv updates;

  timeutil::ptime t = time.t0();
  for (; t < time.t1(); t += step) {
    TaskUpdate_p tu = createU(da, SensorTime(sensor, t));
    Helpers::changeControlinfo(tu, fc_miss);
    tu->clearTasks(ALL_RR24_TASKS);
    updates.push_back(tu);
  }
  ObsData_p obs = da->findE(SensorTime(sensor, t));
  const bool newEndpoint = obs and (is_endpoint(obs) == NO);

  TaskUpdate_p tu = createU(da, SensorTime(sensor, t));
  const float obs_orig = obs ? obs->original() : tu->corrected();

  Helpers::changeControlinfo(tu, fc_end);
  if (tu->corrected() == -1 and obs_orig == -1)
    Helpers::changeControlinfo(tu, fc_dryEnd);
  else
    Helpers::changeControlinfo(tu, fc_wetEnd);
  tu->clearTasks(ALL_RR24_TASKS);

  da->storeUpdates(updates);

  // mark all accumulation rows around period with task
  markPreviousAccumulation(da, sensor, TimeSpan(editableTime.t0(), time.t0()-step), true);
  if( newEndpoint )
    markPreviousAccumulation(da, sensor, TimeSpan(t+step, editableTime.t1()), false);
}

// ========================================================================

namespace /*anonymous*/ {
inline float dry2real(float original)
{ return Helpers::float_eq()(original, -1.0f) ? 0.0f : original; }

// ------------------------------------------------------------------------

inline float real2dry(float real)
{ return Helpers::float_eq()(real, 0.0f) ? -1.0f : real; }

} //namespace anonymous

bool redistributeProposal(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time, const ObsPgmRequest* op, float_v& values)
{
  METLIBS_LOG_SCOPE();
  using namespace Helpers;
  const boost::gregorian::date_duration step = boost::gregorian::days(1);

  float accumulated = 0;
  for (float_v::const_iterator it = values.begin(); it != values.end(); ++it) {
    const float v = dry2real(*it);
    if (v >= 0)
      accumulated += v;
  }
  if (accumulated == 0) {
    values = float_v(values.size(), -1.0f);
    return true;
  }

  // see QC2 RedistributionAlgorithm::redistributePrecipitation

  const FlagPattern neighbor_flags_ci("fd=1", FlagPattern::CONTROLINFO);
  const int MAX_NEIGHBORS = 5;
  const float WARN_DIST_NEIGHBORS = 50.0f, // km
      WARN_DIST_NEIGHBORS2 = WARN_DIST_NEIGHBORS*WARN_DIST_NEIGHBORS;
  const float MAX_DIST_NEIGHBORS = 100.0f; // km

  typedef std::vector<Sensor> Sensor_v;
  Sensor_v neighbors;
  KvMetaDataBuffer::instance()->addNeighbors(neighbors, sensor, time, op, 10*MAX_NEIGHBORS);

  std::vector<float> distances;
  { Helpers::stations_by_distance center(KvMetaDataBuffer::instance()->findStation(sensor.stationId));
    for (Sensor_v::const_iterator itN = neighbors.begin(); itN != neighbors.end(); ++itN)
      distances.push_back(center.distance(KvMetaDataBuffer::instance()->findStation(itN->stationId)));
  }
          
  float weightedNeighborsAccumulated = 0;
  std::vector<float> weightedNeighbors;

  for (timeutil::ptime t = time.t0(); t <= time.t1(); t += step) {
    float sumWeights = 0.0, sumWeightedValues = 0.0;
    int iN = 0, usedNeighbors = 0, lastStation = 0;
    for (Sensor_v::const_iterator itN = neighbors.begin(); itN != neighbors.end(); ++itN, ++iN) {
      METLIBS_LOG_DEBUG(LOGVAL(*itN) << LOGVAL(t));
      if (itN->stationId == lastStation or itN->typeId != sensor.typeId)
        continue;

      ObsData_p nobs = da->findE(SensorTime(*itN, t));
      if (not nobs)
        continue;

      if (not (neighbor_flags_ci.matches(nobs->controlinfo())
              and Helpers::extract_ui2(nobs) == 0))
        continue;

      const float orig = dry2real(nobs->original());
      if (orig <= -2.0f)
        continue;
      const float d = distances[iN];
      if (d >= MAX_DIST_NEIGHBORS)
        break;

      lastStation = itN->stationId;

      const float weight = WARN_DIST_NEIGHBORS2/(d*d);
      METLIBS_LOG_DEBUG(LOGVAL(weight) << LOGVAL(d));
      sumWeights += weight;
      sumWeightedValues += weight*orig;
      usedNeighbors += 1;
      if (usedNeighbors >= MAX_NEIGHBORS)
        break;
    }
    if (usedNeighbors == 0)
      return false;
    const float wn = (sumWeights > 0.0f)
        ? sumWeightedValues/sumWeights : -1.0f;
    METLIBS_LOG_DEBUG(LOGVAL(sumWeightedValues) << LOGVAL(sumWeights) << LOGVAL(wn));
    weightedNeighbors.push_back(wn);
    weightedNeighborsAccumulated += std::max(0.0f, wn);
  }

  const float scale = (weightedNeighborsAccumulated > 0.0f)
      ? accumulated / weightedNeighborsAccumulated : 0.0f;
  METLIBS_LOG_DEBUG(LOGVAL(scale));
  
  for (size_t iT = 0; iT < weightedNeighbors.size(); ++iT) {
    values[iT] = (weightedNeighbors[iT] > 0.05)
        ? Helpers::roundDecimals(scale * weightedNeighbors[iT], 1) : -1.0f;
  }

  return true;
}

// ========================================================================

bool canRedistributeInQC2(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time)
{
  if (time.days() < 1)
    return false;

  const boost::gregorian::date_duration step = boost::gregorian::days(1);
  const FlagPattern ok_miss("fmis=[13]", FlagPattern::CONTROLINFO),
      ok_end("fmis=[024]", FlagPattern::CONTROLINFO);

  int acc_type = -1;
  for (timeutil::ptime t = time.t0(); t < time.t1(); t += step) {
    ObsData_p obs = da->findE(SensorTime(sensor, t));
    if (not obs)
      continue;
    if (not ok_miss.matches(obs->controlinfo()))
      return false;
    if (acc_type < 0)
      acc_type = Helpers::is_accumulation(obs);
    else if (acc_type != Helpers::is_accumulation(obs))
      return false;
  }
  ObsData_p obs = da->findE(SensorTime(sensor, time.t1()));
  if (not obs or not ok_end.matches(obs->controlinfo()) or obs->original() < -1.0f)
    return false;
  if (acc_type >= 0 and acc_type != Helpers::is_accumulation(obs))
    return false;
  return true;
}

// ========================================================================

void singles(TaskAccess_p da, const Sensor& sensor, const timeutil::ptime& t0, const TimeSpan& editableTime,
    const std::vector<float>& newCorrected, const std::vector<int>& acceptReject)
{
  METLIBS_LOG_SCOPE();
  using namespace Helpers;
  const boost::gregorian::date_duration step = boost::gregorian::days(1);
  const FlagChange fc_clear_fd("fd=1");

  da->newVersion();

  bool previousWasSingle = false, previousWasAcc = false;
  unsigned int editRow = 0;
  timeutil::ptime t = t0, tMarkStart = editableTime.t0();
  for (; editRow < newCorrected.size(); t += step, editRow += 1) {
    METLIBS_LOG_DEBUG(LOGVAL(t) << LOGVAL(previousWasSingle) << LOGVAL(previousWasAcc));
    const SensorTime st(sensor, t);
    const int ar = acceptReject.at(editRow);

    if (ar == AR_ACCEPT or ar == AR_REJECT) {
      ObsData_p obs = da->findE(st);
      const bool accumulation = (obs and is_accumulation(obs));
      if (not previousWasSingle) {
        if (accumulation)
          markPreviousAccumulation(da, sensor, TimeSpan(tMarkStart, t-step), true);
        if (previousWasAcc)
          markPreviousAccumulation(da, sensor, TimeSpan(tMarkStart, t-step), false);
      }
      previousWasAcc = (accumulation and not is_endpoint(obs));
      previousWasSingle = true;

      TaskUpdate_p u = createU(da, st);
      if (ar == AR_ACCEPT)
        Helpers::auto_correct(u, obs, newCorrected.at(editRow));
      else
        Helpers::reject(u, obs);
      Helpers::changeControlinfo(u, fc_clear_fd);
      u->clearTasks(ALL_RR24_TASKS);
      da->storeUpdates(ObsUpdate_pv(1, u));
    } else if (previousWasSingle) {
      tMarkStart = t;
      METLIBS_LOG_DEBUG(LOGVAL(tMarkStart));
      previousWasSingle = false;
    }
  }
  if (previousWasSingle) {
    tMarkStart = t;
    METLIBS_LOG_DEBUG(LOGVAL(tMarkStart));
  }

  // mark all accumulation rows around period with task
  if (previousWasAcc)
    markPreviousAccumulation(da, sensor, TimeSpan(tMarkStart, editableTime.t1()), false);
}

// ========================================================================

float calculateSum(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time)
{
  const boost::gregorian::date_duration step = boost::gregorian::days(1);

  float sum = 0;
  for (timeutil::ptime t = time.t0(); t <= time.t1(); t += step) {
    ObsData_p obs = da->findE(SensorTime(sensor, t));
    if (not obs or Helpers::is_missing(obs))
      continue;
    sum += std::max(0.0f, obs->corrected());
  }
  return sum;
}

// ========================================================================

float calculateOriginalSum(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time)
{
  if (time.days() < 1)
    return kvalobs::MISSING;

  const boost::gregorian::date_duration step = boost::gregorian::days(1);
  for (timeutil::ptime t = time.t0(); t < time.t1(); t += step) {
    ObsData_p obs = da->findE(SensorTime(sensor, t));
    if (not obs)
      continue;
    if (not Helpers::is_orig_missing(obs))
      return kvalobs::MISSING;
  }
  ObsData_p obs = da->findE(SensorTime(sensor, time.t1()));
  if (not obs or Helpers::is_orig_missing(obs))
    return kvalobs::MISSING;
  return std::max(0.0f, obs->original());
}

// ========================================================================

bool canAccept(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time, bool corrected)
{
  METLIBS_LOG_SCOPE();
  const boost::gregorian::date_duration step = boost::gregorian::days(1);

  int possible = corrected ? AcceptReject::CAN_ACCEPT_CORRECTED : AcceptReject::CAN_ACCEPT_ORIGINAL;
  const FlagPattern acceptable("fhqc=[01234]", FlagPattern::CONTROLINFO);
  for (timeutil::ptime t = time.t0(); t <= time.t1(); t += step) {
    ObsData_p obs = da->findE(SensorTime(sensor, t));
    if (not obs)
      continue;
    if (not acceptable.matches(obs->controlinfo()))
      return false;
    if (TaskData_p tobs = std::dynamic_pointer_cast<TaskData>(obs)) {
      if ((tobs->allTasks() & ALL_RR24_TASKS) != 0)
        return false;
    }
    possible &= AcceptReject::possibilities(obs, true);
  }
  return possible != 0;
}

// ========================================================================

void accept(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time, bool corrected)
{
  METLIBS_LOG_SCOPE();
  const boost::gregorian::date_duration step = boost::gregorian::days(1);

  da->newVersion();

  for (timeutil::ptime t = time.t0(); t <= time.t1(); t += step) {
    const SensorTime st(sensor, t);
    ObsData_p obs = da->findE(st);
    if (not obs)
      continue;
    if (corrected)
      AcceptReject::accept_corrected(da, obs, false);
    else
      AcceptReject::accept_original(da, obs);
  }
}

} // namespace RR24
