
#include "ObsRequest.hh"

#include "ObsData.hh"

#include <QMetaType>

#define MILOGGER_CATEGORY "kvhqc.ObsRequest"
#include "common/ObsLogging.hh"

LOG_CONSTRUCT_COUNTER;

static bool initMetaType = false;

ObsRequest::ObsRequest()
{
  LOG_CONSTRUCT();
  if (not initMetaType) {
    qRegisterMetaType<ObsRequest_p>("ObsRequest_p");

    qRegisterMetaType<ObsData_pv>("ObsData_pv");
    initMetaType = true;
  }
}

ObsRequest::~ObsRequest()
{
  LOG_DESTRUCT();
}

void ObsRequest::completed(const QString& withError)
{
  Q_EMIT requestCompleted(withError);
}

