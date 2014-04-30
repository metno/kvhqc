
#include "SignalRequest.hh"

#include "BaseRequest.hh"

#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.SignalRequest"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

class DeadRequest : public BaseRequest
{
public:
  DeadRequest(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
    : BaseRequest(sensors, timeSpan, filter) { }
      
  virtual void completed(bool)
    { METLIBS_LOG_SCOPE(); }

  virtual void newData(const ObsData_pv&)
    { METLIBS_LOG_SCOPE(); }

  virtual void updateData(const ObsData_pv&)
    { METLIBS_LOG_SCOPE(); }

  virtual void dropData(const SensorTime_v&)
    { METLIBS_LOG_SCOPE(); }
};

} // namespace anonymous

// ========================================================================

SignalRequest::SignalRequest(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
  : WrapRequest(boost::make_shared<DeadRequest>(sensors, timeSpan, filter))
{
  METLIBS_LOG_SCOPE();
}

SignalRequest::SignalRequest(ObsRequest_p wrapped)
  : WrapRequest(wrapped)
{
  METLIBS_LOG_SCOPE();
}

void SignalRequest::completed(bool failed)
{
  METLIBS_LOG_SCOPE();
  wrapped()->completed(failed);
  Q_EMIT requestCompleted(failed);
}

void SignalRequest::newData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  wrapped()->newData(data);
  Q_EMIT requestNewData(data);
}

void SignalRequest::updateData(const ObsData_pv& data)
{
  METLIBS_LOG_SCOPE();
  wrapped()->updateData(data);
  Q_EMIT requestUpdateData(data);
}

void SignalRequest::dropData(const SensorTime_v& dropped)
{
  METLIBS_LOG_SCOPE();
  wrapped()->dropData(dropped);
  Q_EMIT requestDropData(dropped);
}
