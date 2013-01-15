
#include "EditData.hh"

#include "Helpers.hh"

EditData::EditData(ObsDataPtr data)
    : mData(data)
    , mCreated(false)
    , mOriginal(mData->original())
{
    const int FIRST_UPDATE = -1;

    int bTasks = 0;
    if (EditDataPtr bebs = boost::dynamic_pointer_cast<EditData>(mData))
        bTasks = bebs->allTasks();
    mTasks.push_back(std::make_pair(FIRST_UPDATE, bTasks));

    mCorrected  .push_back(std::make_pair(FIRST_UPDATE, mData->corrected()));
    mControlinfo.push_back(std::make_pair(FIRST_UPDATE, mData->controlinfo()));
}

bool EditData::modified() const
{
    return (modifiedCorrected() or modifiedControlinfo());
}

bool EditData::modifiedCorrected() const
{
    return (mCreated or mCorrected.size() > 1) and not Helpers::float_eq()(oldCorrected(), corrected());
}

bool EditData::modifiedControlinfo() const
{
    return (mCreated or mControlinfo.size() > 1) and (oldControlinfo() != controlinfo());
}

bool EditData::updateFromBackend()
{
    const float bOriginal = mData->original();
    bool changed = (not Helpers::float_eq()(mOriginal, bOriginal));
    mOriginal = bOriginal;

    int bTasks = 0;
    if (EditDataPtr bebs = boost::dynamic_pointer_cast<EditData>(mData))
        bTasks = bebs->allTasks();
    changed |= (bTasks != mTasks.front().second);
    mTasks.front().second = bTasks;

    const float bCorrected = mData->corrected();
    changed |= (not Helpers::float_eq()(corrected(), bCorrected));
    mCorrected.front().second = bCorrected;

    const kvalobs::kvControlInfo& bControlinfo = mData->controlinfo();
    changed |= (controlinfo() != bControlinfo);
    mControlinfo.front().second = bControlinfo;

    return changed;
}
