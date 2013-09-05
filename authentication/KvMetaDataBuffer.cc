
#include "KvMetaDataBuffer.hh"

#include "BusyIndicator.h"
#include "Functors.hh"
#include "HqcApplication.hh"
#include "KvServiceHelper.hh"

#include <puTools/miStringBuilder.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QVariant>
#include <QtGui/QMessageBox>
#include <QtSql/QSqlQuery>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvMetaDataBuffer"
#define M_TIME
#include "HqcLogging.hh"

KvMetaDataBuffer* KvMetaDataBuffer::sInstance = 0;

KvMetaDataBuffer::KvMetaDataBuffer()
    : mHaveStations(false)
    , mHaveParams(false)
{
  assert(not sInstance);
  sInstance = this;
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
      throw std::runtime_error(miutil::StringBuilder() << "station " << id << "' not found");
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
      throw std::runtime_error(miutil::StringBuilder() << "param '" << id << " not found");
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

  if (hqcApp) {
    QSqlQuery query(hqcApp->systemDB());
    query.exec("SELECT low, high FROM slimits WHERE paramid = ?");
    query.bindValue(0, paramid);
    query.exec();
    if (query.next()) {
      const float low = query.value(0).toFloat(), high = query.value(1).toFloat();
      METLIBS_LOG_DEBUG(LOGVAL(paramid) << LOGVAL(low) << LOGVAL(value) << LOGVAL(high));
      return (low <= value and high >= value);
    }
  }

  METLIBS_LOG_DEBUG("no limits for paramid " << paramid);
  return true;
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
      throw std::runtime_error(miutil::StringBuilder() << "type " << id << " not found");
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
          KvServiceHelper::instance()->getKvObsPgm(mObsPgms[stationid], stations);
        } catch (std::exception& e) {
          METLIBS_LOG_ERROR("exception while retrieving obs_pgm for station " << stationid << ": " << e.what());
        }
    }
    return mObsPgms[stationid];
}

void KvMetaDataBuffer::findObsPgm(const std::set<long>& stationids)
{
  METLIBS_LOG_TIME();
  const std::set<long> stationsLoaded(boost::adaptors::keys(mObsPgms).begin(), boost::adaptors::keys(mObsPgms).end());
  std::vector<long> stationsToFetch;
  std::set_difference(stationids.begin(), stationids.end(), stationsLoaded.begin(), stationsLoaded.end(),
      std::back_inserter(stationsToFetch));

  if (not stationsToFetch.empty()) {
    METLIBS_LOG_DEBUG(LOGVAL(stationids.size()) << LOGVAL(stationsToFetch.size()));
    
    // obs_pgm is too large for a single CORBA reply, split in chunks of 100 stations
    const size_t total = stationsToFetch.size(), chunk = 100;
    size_t start = 0;
    std::vector<long>::const_iterator f0 = stationsToFetch.begin(), f1 = f0;
    while (f0 != stationsToFetch.end()) {
      const size_t n = std::min(total-start, chunk);
      start += n;
      f1    += n;
      const std::list<long> chunkIds(f0, f1);
      f0 = f1;

      try {
        ObsPgmList mixed;
        KvServiceHelper::instance()->getKvObsPgm(mixed, chunkIds);
          
        ObsPgmList::const_iterator i0 = mixed.begin(), i1 = i0;
        while (i0 != mixed.end()) {
          const long s0 = i0->stationID();
          while (i1 != mixed.end() and i1->stationID() == s0)
            ++i1;
          ObsPgmList& l0 = mObsPgms[s0];
          l0.insert(l0.end(), i0, i1);
          i0 = i1;
        }
      } catch (std::exception& e) {
        METLIBS_LOG_ERROR("exception retrieving obs_pgm for station list chunk");
      }
    }
  }
}

void KvMetaDataBuffer::fetchStations()
{
    METLIBS_LOG_SCOPE();
    BusyIndicator wait;
    mHaveStations = true;
    mStations.clear();
    try {
      if (not KvServiceHelper::instance()->getKvStations(mStations))
        METLIBS_LOG_ERROR("could not fetch station list");
    } catch (std::exception& e) {
      METLIBS_LOG_ERROR("exception while retrieving station list: " << e.what());
    }
}

void KvMetaDataBuffer::fetchParams()
{
    METLIBS_LOG_SCOPE();
    BusyIndicator wait;
    mHaveParams = true;
    mParams.clear();
    try {
      if (not KvServiceHelper::instance()->getKvParams(mParams))
        METLIBS_LOG_ERROR("could not fetch param list");
    } catch (std::exception& e) {
      METLIBS_LOG_ERROR("exception while retrieving param list: " << e.what());
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
    METLIBS_LOG_SCOPE();
    BusyIndicator wait;
    mHaveTypes = true;
    mTypes.clear();
    try {
      if (not KvServiceHelper::instance()->getKvTypes(mTypes))
        METLIBS_LOG_ERROR("could not fetch type list");
    } catch (std::exception& e) {
      METLIBS_LOG_ERROR("exception while retrieving param list: " << e.what());
    }
}

void KvMetaDataBuffer::reload()
{
    mHaveStations = mHaveParams = mHaveTypes = false;
    mObsPgms.clear();
}
