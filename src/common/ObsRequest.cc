
#include "ObsRequest.hh"

#include "ObsData.hh"

#include <QtCore/QMetaType>

static bool initMetaType = false;

ObsRequest::ObsRequest()
  : mTag(0)
{
  if (not initMetaType) {
    qRegisterMetaType<ObsRequest_p>("ObsRequest_p");

    qRegisterMetaType<ObsData_pv>("ObsData_pv");
    initMetaType = true;
  }
}

ObsRequest::~ObsRequest()
{
}
