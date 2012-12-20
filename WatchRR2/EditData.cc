
#include "EditData.hh"

#include "Helpers.hh"

bool EditData::modified() const
{
    if (mCreated)
        return true;
    if (not mNewCorrected.empty())
        return not Helpers::float_eq()(oldCorrected(), mNewCorrected.back().second);
    if (not mNewControlinfo.empty())
        return oldControlinfo() != mNewControlinfo.back().second;
    return false;
}
