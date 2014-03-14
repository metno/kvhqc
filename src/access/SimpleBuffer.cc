
#include "SimpleBuffer.hh"

#include "SimpleRequest.hh"
#include "SignalRequest.hh"
#include "SyncRequest.hh"

#define MILOGGER_CATEGORY "kvhqc.SimpleBuffer"
#include "common/ObsLogging.hh"

SimpleBuffer::SimpleBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
  : mRequest(new SimpleRequest(this, sensors, timeSpan, filter))
  , mComplete(INCOMPLETE) // FIXME
{
  METLIBS_LOG_SCOPE();
}

SimpleBuffer::SimpleBuffer(SignalRequest_p request)
  : mRequest(request)
  , mComplete(INCOMPLETE) // FIXME
{
  METLIBS_LOG_SCOPE();
  SignalRequest* r = request.get();
  connect(r, SIGNAL(requestCompleted(bool)),               this, SLOT(completed(bool)));
  connect(r, SIGNAL(requestNewData(const ObsData_pv&)),    this, SLOT(newData(const ObsData_pv&)));
  connect(r, SIGNAL(requestUpdateData(const ObsData_pv&)), this, SLOT(updateData(const ObsData_pv&)));
  connect(r, SIGNAL(requestDropData(const SensorTime_v&)), this, SLOT(dropData(const SensorTime_v&)));
}

SimpleBuffer::~SimpleBuffer()
{
  METLIBS_LOG_SCOPE();
  if (mAccess)
    mAccess->dropRequest(mRequest);
}

void SimpleBuffer::postRequest(ObsAccess_p access)
{
  METLIBS_LOG_SCOPE();
  mAccess = access;
  mAccess->postRequest(mRequest);
}

void SimpleBuffer::syncRequest(ObsAccess_p access)
{
  METLIBS_LOG_SCOPE();
  mAccess = access;
  mRequest = ::syncRequest(mRequest, mAccess);
}

void SimpleBuffer::completed(bool failed)
{
  METLIBS_LOG_SCOPE();
  if (failed)
    mComplete = FAILED;
  else
    mComplete = COMPLETE;
  Q_EMIT bufferCompleted(failed);
  METLIBS_LOG_DEBUG(LOGVAL(failed));
}

void SimpleBuffer::newData(const ObsData_pv& data)
{
  Q_EMIT newDataBegin();
  onNewData(data);
  Q_EMIT newDataEnd(data);
}

void SimpleBuffer::updateData(const ObsData_pv& data)
{
  Q_EMIT updateDataBegin();
  onUpdateData(data);
  Q_EMIT updateDataEnd(data);
}

void SimpleBuffer::dropData(const SensorTime_v& dropped)
{
  Q_EMIT dropDataBegin();
  onDropData(dropped);
  Q_EMIT dropDataEnd(dropped);
}
