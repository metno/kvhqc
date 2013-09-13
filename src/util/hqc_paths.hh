
#ifndef HQC_PATHS
#define HQC_PATHS 1

#include <QString>

namespace hqc {

enum PathId {
    CONFDIR = 0,
    DATADIR,
    IMAGEDIR,
    DOCDIR,
    PATHID__END
};


QString getPath(PathId id);

} // namespace hqc

#endif /* HQC_PATHS */
