
#include "KvModelAccess.hh"
#include "Helpers.hh"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define NDEBUG
#include "w2debug.hh"

ModelDataPtr KvModelAccess::find(const SensorTime& st)
{
    Data_t::iterator it = mData.find(st);
    if (it != mData.end())
        return it->second;

    return KvalobsModelDataPtr();
}

KvalobsModelDataPtr KvModelAccess::receive(const kvalobs::kvModelData& data)
{
    const SensorTime st(Helpers::sensorTimeFromKvModelData(data)); 

    KvalobsModelDataPtr mdl;
    Data_t::iterator it = mData.find(st);
    if (it == mData.end()) {
        mdl = boost::make_shared<KvalobsModelData>(data);
        mData[st] = mdl;
        modelDataChanged(mdl);
    } else {
        mdl = it->second;

        // TODO this might compare too many things ...
        if (mdl->data() != data) {
            mdl->data() = data;
            modelDataChanged(mdl);
        }
    }
    return mdl;
}

bool KvModelAccess::drop(const SensorTime& st)
{
    Data_t::iterator it = mData.find(st);
    if (it == mData.end())
        return false;

    ModelDataPtr mdl = it->second;
    mData.erase(it);
    modelDataChanged(mdl);
    return true;
}