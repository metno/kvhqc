
#ifndef ObsUpdate_hh
#define ObsUpdate_hh 1

#include "ObsData.hh"

struct ObsUpdate {
    ObsDataPtr obs;
    float corrected;
    kvalobs::kvControlInfo controlinfo;
    int tasks;
    ObsUpdate(ObsDataPtr o, float c, const kvalobs::kvControlInfo& ci, int t)
        : obs(o), corrected(c), controlinfo(ci), tasks(t) { }
};

#endif // ObsUpdate_hh
