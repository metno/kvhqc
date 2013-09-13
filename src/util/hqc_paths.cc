
#include "hqc_paths.hh"
#include "config.h"

#include <cstdlib>

namespace {

const char* hqc_pathnames[hqc::PATHID__END] = {
    "HQC_CONFDIR",
    "HQC_DATADIR",
    "HQC_IMAGEDIR",
    "HQC_DOCDIR"
};

#define DATADIR PREFIX "/share/kvhqc/" PVERSION
#define DOCDIR  PREFIX "/share/doc/kvhqc-" PVERSION

const char* hqc_defaultpaths[hqc::PATHID__END] = {
    SYSCONFDIR "/kvhqc/" PVERSION,
    DATADIR,
    DATADIR "/images",
    DOCDIR
};

} // anonymous namespace

namespace hqc {

QString getPath(PathId id)
{
    const char* env = getenv(hqc_pathnames[id]);
    if( env )
        return env;
    return hqc_defaultpaths[id];
}

} // namespace hqc
