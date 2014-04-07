
#ifndef CountDataChanged_hh
#define CountDataChanged_hh 1

#include "ObsAccess.hh"
#include <boost/noncopyable.hpp>

struct CountDataChanged : private boost::noncopyable
{
    int count;
    ObsAccess::ObsDataChange filterWhat;
    int filterParam;
    CountDataChanged();
    void operator()(ObsAccess::ObsDataChange what, ObsDataPtr obs);
};

#endif // CountDataChanged_hh
