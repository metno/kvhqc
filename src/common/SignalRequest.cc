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


#include "SignalRequest.hh"

#include "BaseRequest.hh"

#define MILOGGER_CATEGORY "kvhqc.SignalRequest"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {

class DeadRequest : public BaseRequest
{
public:
  DeadRequest(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
    : BaseRequest(sensors, timeSpan, filter) { }

  virtual void newData(const ObsData_pv&) override { METLIBS_LOG_SCOPE(); }

  virtual void updateData(const ObsData_pv&) override { METLIBS_LOG_SCOPE(); }

  virtual void dropData(const SensorTime_v&) override { METLIBS_LOG_SCOPE(); }
};

} // namespace anonymous

// ========================================================================

SignalRequest::SignalRequest(const Sensor_s& sensors, const TimeSpan& timeSpan, ObsFilter_p filter)
    : WrapRequest(std::make_shared<DeadRequest>(sensors, timeSpan, filter))
{
  METLIBS_LOG_SCOPE();
}

SignalRequest::SignalRequest(ObsRequest_p wrapped)
  : WrapRequest(wrapped)
{
  METLIBS_LOG_SCOPE();
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
