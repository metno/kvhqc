
#include "hqc_paths.hh"
#include "config.h"

#include <cstdlib>

namespace {

const char* hqc_pathnames[hqc::PATHID__END] = {
  "HQC_CONFDIR",
  "HQC_DATADIR",
  "HQC_IMAGEDIR",
  "HQC_DOCDIR",
  "HQC_LANGDIR",
};

const char* hqc_defaultpaths[hqc::PATHID__END] = {
  SYSCONFDIR "/kvhqc/" PVERSION,
  PREFIX "/share/kvhqc/" PVERSION,
  0,
  PREFIX "/share/doc/kvhqc-" PVERSION,
  0
};

const char* hqc_defaultdatapaths[hqc::PATHID__END] = {
  0,
  0,
  "/images",
  0,
  "/lang"
};

} // anonymous namespace

namespace hqc {

QString getPath(PathId id)
{
    const char* env = getenv(hqc_pathnames[id]);
    if (env)
      return env;
    const char* d = hqc_defaultpaths[id];
    if (d)
      return d;
    return getPath(DATADIR) + hqc_defaultdatapaths[id];
}

} // namespace hqc
