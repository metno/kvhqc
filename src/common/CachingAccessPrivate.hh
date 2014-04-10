
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
  BackendBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter);
  ~BackendBuffer();

  void use()
    { mUseCount += 1; }
  
  void drop();
  
  bool isUnused() const
    { return mUseCount == 0; }

  const Time& unusedSince() const
    { return mUnusedSince; }

  const Sensor_s& sensors() const
    { return request()->sensors(); }

  const TimeSpan& timeSpan() const
    { return request()->timeSpan(); }

  ObsFilter_p filter() const
    { return request()->filter(); }

  const ObsDataByTime_ps& data() const
    { return TimeBuffer::data(); }

private:
  size_t mUseCount;
  Time mUnusedSince;
};

HQC_TYPEDEF_X(BackendBuffer);
HQC_TYPEDEF_P(BackendBuffer);
HQC_TYPEDEF_PV(BackendBuffer);
HQC_TYPEDEF_PL(BackendBuffer);

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
    { ObsFilter_p f = filter(); return (not f) or f->accept(obs, false); }

  ObsFilter_p filter() const
    { return mRequest->filter(); }

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
  bool operator()(const Time& ta, BackendBuffer_p b) const
    { return ta < b->timeSpan().t0(); }
  bool operator()(BackendBuffer_p a, const Time& tb) const
    { return a->timeSpan().t0() < tb; }
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

  BackendBuffer_p create(const Sensor_s& sensors, const TimeSpan& time, ObsFilter_p filter);
  void clean(const Time& dropBefore);

  ObsAccess_p backend;
  BackendBuffer_pl mBuffers;
};

#endif // ACCESS_CACHINGACCESSPRIVATE_HH
