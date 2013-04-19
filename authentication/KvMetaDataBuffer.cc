
#include "KvMetaDataBuffer.hh"
#include "hqc_paths.hh"
#include "BusyIndicator.h"
#include "Functors.hh"

#include <kvcpp/KvApp.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QMessageBox>

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#define NDEBUG
#include "debug.hh"

KvMetaDataBuffer* KvMetaDataBuffer::sInstance = 0;

KvMetaDataBuffer::KvMetaDataBuffer()
    : mHaveStations(false)
    , mHaveParams(false)
{
    assert(not sInstance);
    sInstance = this;

    QString limitsFile = ::hqc::getPath(::hqc::CONFDIR) + "/slimits";
    QFile limits(limitsFile);
    if (not limits.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0, qApp->translate("ErrorList", "Cannot open file"),
                              qApp->translate("ErrorList", "Could not open file '%1' for reading. Have to exit kvhqc.").arg(limitsFile),
                              QMessageBox::Ok, QMessageBox::NoButton);
        exit(1);
    }
    
    QTextStream limitStream(&limits);
    while (not limitStream.atEnd()) {
        int par, dum;
        float low, high;
        limitStream >> par >> dum >> low >> high;
        mParamLimits[par] = std::make_pair(low, high);
    }
}

KvMetaDataBuffer::~KvMetaDataBuffer()
{
    sInstance = 0;
}

bool KvMetaDataBuffer::isKnownStation(int id)
{
    if (not mHaveStations)
        fetchStations();

    std::list<kvalobs::kvStation>::const_iterator it
        = std::find_if(mStations.begin(), mStations.end(), Helpers::station_by_id(id));
    return (it != mStations.end());
}

const kvalobs::kvStation& KvMetaDataBuffer::findStation(int id)
{
    if (not mHaveStations)
        fetchStations();

    std::list<kvalobs::kvStation>::const_iterator it
        = std::find_if(mStations.begin(), mStations.end(), Helpers::station_by_id(id));
    if (it == mStations.end())
        throw std::runtime_error("station not found");
    return *it;
}

const std::list<kvalobs::kvStation>& KvMetaDataBuffer::allStations()
{
    if (not mHaveStations)
        fetchStations();
    return mStations;
}

bool KvMetaDataBuffer::isKnownParam(int id)
{
    if (not mHaveParams)
        fetchParams();

    std::list<kvalobs::kvParam>::const_iterator it
        = std::find_if(mParams.begin(), mParams.end(), Helpers::param_by_id(id));
    return (it != mParams.end());
}

const kvalobs::kvParam& KvMetaDataBuffer::findParam(int id)
{
    if (not mHaveParams)
        fetchParams();

    std::list<kvalobs::kvParam>::const_iterator it
        = std::find_if(mParams.begin(), mParams.end(), Helpers::param_by_id(id));
    if (it == mParams.end())
        throw std::runtime_error("param not found");
    return *it;
}

const std::list<kvalobs::kvParam>& KvMetaDataBuffer::allParams()
{
    if (not mHaveParams)
        fetchParams();
    return mParams;
}

std::string KvMetaDataBuffer::findParamName(int paramId)
{
    if (not mHaveParams)
        fetchParams();

    std::list<kvalobs::kvParam>::const_iterator it
        = std::find_if(mParams.begin(), mParams.end(), Helpers::param_by_id(paramId));
    if (it != mParams.end())
        return it->name();
    
    return (boost::format("{%1$d}") % paramId).str();
}

bool KvMetaDataBuffer::isCodeParam(int paramid)
{
    if (not mHaveParams)
        fetchParams();
    return mCodeParams.find(paramid) != mCodeParams.end();
}

namespace /* anonymous */ {
const int NOPARAMMODEL     = 8;
const int modelParam[NOPARAMMODEL] =
{ 61, 81, 109, 110, 177, 178, 211, 262 };
} // anonymous namespace

bool KvMetaDataBuffer::isModelParam(int paramid)
{
    return std::binary_search(modelParam, boost::end(modelParam), paramid);
}

bool KvMetaDataBuffer::checkPhysicalLimits(int paramid, float value)
{
    if (value == -32767 or value == -32766)
        return true;

    ParamLimits_t::const_iterator it = mParamLimits.find(paramid);
    if (it == mParamLimits.end()) {
        LOG4HQC_DEBUG("KvMetaDataBuffer", "no limits for paramid " << paramid);
        return true;
    }
    return (it->second.first <= value and it->second.second >= value);
}

bool KvMetaDataBuffer::isKnownType(int id)
{
    if (not mHaveTypes)
        fetchTypes();

    std::list<kvalobs::kvTypes>::const_iterator it
        = std::find_if(mTypes.begin(), mTypes.end(), Helpers::type_by_id(id));
    return (it != mTypes.end());
}

const kvalobs::kvTypes& KvMetaDataBuffer::findType(int id)
{
    if (not mHaveTypes)
        fetchTypes();

    std::list<kvalobs::kvTypes>::const_iterator it
        = std::find_if(mTypes.begin(), mTypes.end(), Helpers::type_by_id(id));
    if (it == mTypes.end())
        throw std::runtime_error("type not found");
    return *it;
}

const std::list<kvalobs::kvTypes>& KvMetaDataBuffer::allTypes()
{
    if (not mHaveTypes)
        fetchTypes();
    return mTypes;
}

const KvMetaDataBuffer::ObsPgmList& KvMetaDataBuffer::findObsPgm(int stationid)
{
    ObsPgms_t::iterator it = mObsPgms.find(stationid);
    if (it == mObsPgms.end()) {
        std::list<long> stations(1, stationid);
        try {
          kvservice::KvApp::kvApp->getKvObsPgm(mObsPgms[stationid], stations, false);
        } catch (std::exception& e) {
          LOG4HQC_ERROR("KvMetaDataBuffer", "exception while retrieving obs_pgm for station " << stationid << ": " << e.what());
        }
    }
    return mObsPgms[stationid];
}

void KvMetaDataBuffer::fetchStations()
{
    LOG_SCOPE();
    BusyIndicator wait;
    mHaveStations = true;
    mStations.clear();
    try {
      if (not kvservice::KvApp::kvApp->getKvStations(mStations))
        LOG4HQC_ERROR("KvMetaDataBuffer", "could not fetch station list");
    } catch (std::exception& e) {
      LOG4HQC_ERROR("KvMetaDataBuffer", "exception while retrieving station list: " << e.what());
    }
}

void KvMetaDataBuffer::fetchParams()
{
    LOG_SCOPE();
    BusyIndicator wait;
    mHaveParams = true;
    mParams.clear();
    try {
      if (not kvservice::KvApp::kvApp->getKvParams(mParams))
        LOG4HQC_ERROR("KvMetaDataBuffer", "could not fetch param list");
    } catch (std::exception& e) {
      LOG4HQC_ERROR("KvMetaDataBuffer", "exception while retrieving param list: " << e.what());
    }
    mCodeParams.clear();
    BOOST_FOREACH(const kvalobs::kvParam& p, mParams) {
        if (p.unit().find("kode") != std::string::npos) {
            mCodeParams.insert(p.paramID());
        }
    }
}

void KvMetaDataBuffer::fetchTypes()
{
    LOG_SCOPE();
    BusyIndicator wait;
    mHaveTypes = true;
    mTypes.clear();
    try {
      if (not kvservice::KvApp::kvApp->getKvTypes(mTypes))
        LOG4HQC_ERROR("KvMetaDataBuffer", "could not fetch type list");
    } catch (std::exception& e) {
      LOG4HQC_ERROR("KvMetaDataBuffer", "exception while retrieving param list: " << e.what());
    }
}

void KvMetaDataBuffer::reload()
{
    mHaveStations = mHaveParams = mHaveTypes = false;
    mObsPgms.clear();
}
