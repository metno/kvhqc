
#include "SimpleBuffer.hh"

#include "SimpleRequest.hh"
#include "SignalRequest.hh"
#include "SyncRequest.hh"

#define MILOGGER_CATEGORY "kvhqc.SimpleBuffer"
#include "common/ObsLogging.hh"

LOG_CONSTRUCT_COUNTER;

SimpleBuffer::SimpleBuffer(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
  : mRequest(new SimpleRequest(this, sensors, timeSpan, filter))
  , mComplete(INCOMPLETE) // FIXME
{
  METLIBS_LOG_SCOPE();
  LOG_CONSTRUCT();
}

SimpleBuffer::SimpleBuffer(SignalRequest_p request)
  : mRequest(request)
  , mComplete(INCOMPLETE) // FIXME
{
  METLIBS_LOG_SCOPE();
  LOG_CONSTRUCT();
  SignalRequest* r = request.get();
  connect(r, SIGNAL(requestCompleted(const QString&)),     this, SLOT(completed(const QString&)));
  connect(r, SIGNAL(requestNewData(const ObsData_pv&)),    this, SLOT(newData(const ObsData_pv&)));
  connect(r, SIGNAL(requestUpdateData(const ObsData_pv&)), this, SLOT(updateData(const ObsData_pv&)));
  connect(r, SIGNAL(requestDropData(const SensorTime_v&)), this, SLOT(dropData(const SensorTime_v&)));
}

SimpleBuffer::~SimpleBuffer()
{
  METLIBS_LOG_SCOPE();
  if (mAccess)
    mAccess->dropRequest(mRequest);
  LOG_DESTRUCT();
}

void SimpleBuffer::postRequest(ObsAccess_p access, bool synchronized)
{
  METLIBS_LOG_SCOPE();
  mAccess = access;
  mAccess->postRequest(mRequest, synchronized);
}

void SimpleBuffer::completed(const QString& withError)
{
  METLIBS_LOG_SCOPE(LOGVAL(this));
  if (not withError.isNull())
    mComplete = FAILED;
  else
    mComplete = COMPLETE;
  Q_EMIT bufferCompleted(withError);
  METLIBS_LOG_DEBUG(LOGVAL(withError));
}

void SimpleBuffer::newData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
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
