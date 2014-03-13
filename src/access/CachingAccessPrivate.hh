
#ifndef ACCESS_CACHINGACCESSPRIVATE_HH
#define ACCESS_CACHINGACCESSPRIVATE_HH 1

#include "CachingAccess.hh"

#include "SimpleRequest.hh"
#include "TimeBuffer.hh"

class BackendBuffer;
HQC_TYPEDEF_P(BackendBuffer);
HQC_TYPEDEF_PV(BackendBuffer);

class BackendBuffer : public TimeBuffer
{
public:
  BackendBuffer(const Sensor& sensor, const TimeSpan& timeSpan, ObsFilter_p filter);
  ~BackendBuffer();

  void use()
    { mUseCount += 1; }
  
  void drop();
  
  bool isUnused() const
    { return mUseCount == 0; }

  const Time& unusedSince() const
    { return mUnusedSince; }

  const Sensor& sensor() const
    { return request()->sensor(); }

  const TimeSpan& timeSpan() const
    { return request()->timeSpan(); }

  ObsFilter_p filter() const
    { return request()->filter(); }

  const ObsData_pl& data() const
    { return TimeBuffer::data(); }

private:
  size_t mUseCount;
  Time mUnusedSince;
};

HQC_TYPEDEF_X(BackendBuffer);
HQC_TYPEDEF_P(BackendBuffer);
HQC_TYPEDEF_PV(BackendBuffer);

// ========================================================================

class CacheTag : public QObject
{ Q_OBJECT;
public:
  CacheTag(ObsRequest_p request, BackendBuffer_pv backendBuffers);
  ~CacheTag();

  void checkComplete();
  
private Q_SLOTS:
  void onBackendCompleted(bool);
  void onBackendNewData(const ObsData_pv&);
  void onBackendUpdateData(const ObsData_pv&);
  void onBackendDropData(const SensorTime_v& dropped);
  
private:
  bool acceptFilter(ObsData_p obs) const
    { ObsFilter_p f = filter(); return (not f) or f->accept(obs, true); }

  bool acceptFilter(const SensorTime& st) const
    { ObsFilter_p f = filter(); return (not f) or f->accept(st, true); }

  ObsFilter_p filter() const
    { return mRequest->filter(); }

  bool acceptObs(ObsData_p obs) const;

  ObsData_pv filterData(const ObsData_pv& dataIn);

public:
  ObsRequest_p mRequest;
  BackendBuffer_pv mBackendBuffers;
  size_t mCountIncomplete, mCountFailed;
};

HQC_TYPEDEF_X(CacheTag);

// ========================================================================

struct BackendBuffer_by_t0 {
  bool operator()(BackendBuffer_p a, BackendBuffer_p b) const
    { return a->timeSpan().t0() < b->timeSpan().t0(); }
};

// ========================================================================

struct ObsData_by_time {
  bool operator()(ObsData_p a, ObsData_p b) const
    { return a->sensorTime().time < b->sensorTime().time; }
  bool operator()(ObsData_p a, const Time& b) const
    { return a->sensorTime().time < b; }
  bool operator()(const Time& a, ObsData_p b) const
    { return a < b->sensorTime().time; }
};

// ========================================================================

class CachingAccessPrivate
{
public:
  CachingAccessPrivate(ObsAccess_p backend);
  ~CachingAccessPrivate();

  BackendBuffer_p create(ObsRequest_p request, const TimeSpan& time);
  void clean(const Time& dropBefore);

  ObsAccess_p backend;
  
  typedef std::set<BackendBuffer_p, BackendBuffer_by_t0> Buffer_s;
  typedef std::map<Sensor, Buffer_s, lt_Sensor> Sensor_Buffer_m;
  Sensor_Buffer_m mSensorBuffers;
};

#endif // ACCESS_CACHINGACCESSPRIVATE_HH
