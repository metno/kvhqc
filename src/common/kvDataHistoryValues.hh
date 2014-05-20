
#ifndef KVDATAHISTORYVALUES_HH
#define KVDATAHISTORYVALUES_HH 1

#include "util/timeutil.hh"
#include "util/boostutil.hh"

#include <kvalobs/flag/kvControlInfo.h>
#include <kvalobs/flag/kvUseInfo.h>

#include <string>

struct kvDataHistoryValues {
  timeutil::ptime modificationtime;
  float corrected;
  kvalobs::kvControlInfo controlinfo;
  kvalobs::kvUseInfo useinfo;
  std::string cfailed;
};

HQC_TYPEDEF_V(kvDataHistoryValues);

#endif // KVDATAHISTORYVALUES_HH
