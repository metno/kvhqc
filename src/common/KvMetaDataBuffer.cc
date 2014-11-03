
#include "KvMetaDataBuffer.hh"

#include "Functors.hh"
#include "KvHelpers.hh"
#include "KvServiceHelper.hh"
#include "common/HqcApplication.hh"
#include "util/gui/BusyIndicator.hh"

#include <puTools/miStringBuilder.h>
#include <kvcpp/KvApp.h>
#include <kvalobs/kvStationParam.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QVariant>
#include <QtGui/QMessageBox>
#include <QtSql/QSqlQuery>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvMetaDataBuffer"
#define M_TIME
#include "util/HqcLogging.hh"

KvMetaDataBuffer* KvMetaDataBuffer::sInstance = 0;

KvMetaDataBuffer::KvMetaDataBuffer()
  : mHaveStations(false)
  , mHaveParams(false)
  , mHaveTypes(false)
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
  
  stations_cit it = std::lower_bound(mStations.begin(), mStations.end(), id, Helpers::kvStation_lt_id());
  return (it != mStations.end() and it->stationID() == id);
}

const kvalobs::kvStation& KvMetaDataBuffer::findStation(int id)
{
  if (not mHaveStations)
    fetchStations();
  
  stations_cit it = std::lower_bound(mStations.begin(), mStations.end(), id, Helpers::kvStation_lt_id());
  if (it != mStations.end() and it->stationID() == id)
    return *it;
  throw std::runtime_error(miutil::StringBuilder() << "station " << id << "' not found");
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

  params_cit it = std::lower_bound(mParams.begin(), mParams.end(), id, Helpers::kvParam_lt_id());
  return (it != mParams.end() and it->paramID() == id);
}

const kvalobs::kvParam& KvMetaDataBuffer::findParam(int id)
{
  if (not mHaveParams)
    fetchParams();

  params_cit it = std::lower_bound(mParams.begin(), mParams.end(), id, Helpers::kvParam_lt_id());
  if (it != mParams.end() and it->paramID() == id)
    return *it;
  throw std::runtime_error(miutil::StringBuilder() << "param " << id << "' not found");
}

const std::list<kvalobs::kvParam>& KvMetaDataBuffer::allParams()
{
  if (not mHaveParams)
    fetchParams();
  return mParams;
}

std::string KvMetaDataBuffer::findParamName(int id)
{
  if (not mHaveParams)
    fetchParams();

  params_cit it = std::lower_bound(mParams.begin(), mParams.end(), id, Helpers::kvParam_lt_id());
  if (it != mParams.end() and it->paramID() == id)
    return it->name();
  
  return (boost::format("{%1$d}") % id).str();
}

bool KvMetaDataBuffer::isCodeParam(int paramid)
{
  if (not mHaveParams)
    fetchParams();
  return mCodeParams.find(paramid) != mCodeParams.end();
}

bool KvMetaDataBuffer::isDirectionInDegreesParam(int pid)
{
  const kvalobs::kvParam& param = findParam(pid);
  return (param.unit().find("grader") != std::string::npos
      and pid != kvalobs::PARAMID_MLON
      and pid != kvalobs::PARAMID_MLAT);
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

KvMetaDataBuffer::ParamLimit KvMetaDataBuffer::checkPhysicalLimits(const SensorTime& st, float value)
{
  if (value == kvalobs::MISSING or value == kvalobs::REJECTED)
    return Ok;
          
  float param_max = 0, param_high = 0, param_low = 0, param_min = 0;
  bool have_max = 0, have_high = 0, have_low = 0, have_min = 0;

  QString metadata;
  if (kvservice::KvApp::kvApp) {
    const int day  = st.time.date().day_of_year();
    const int hour = st.time.time_of_day().hours();
    std::list<kvalobs::kvStationParam> stParam;
    if (kvservice::KvApp::kvApp->getKvStationParam(stParam, st.sensor.stationId, st.sensor.paramId, day)) {
      timeutil::ptime recent;
      for (std::list<kvalobs::kvStationParam>::const_iterator it = stParam.begin(); it != stParam.end(); ++it) {
        if ((it->hour() == hour or it->hour() == -1)
            and it->sensor() == st.sensor.sensor
            and it->level() == st.sensor.level
            and st.time >= it->fromtime()
            and (recent.is_not_a_date_time() or it->fromtime() > recent))
        {
          metadata = QString::fromStdString(it->metadata());
          recent = it->fromtime();
        }
      }
    }
  }
  if (not metadata.isNull()) {
    const QStringList lines = metadata.split(QChar('\n'));
    if (lines.length() == 2) {
      const QStringList keys = lines.at(0).split(QChar(';'));
      const QStringList values = lines.at(1).split(QChar(';'));
      if (keys.length() == values.length()) {
        for (int i=0; i<keys.length(); ++i) {
          if (keys.at(i) == "max") {
            param_max = values.at(i).toFloat();
            have_max = true;
          } else if (keys.at(i) == "min") {
            param_min = values.at(i).toFloat();
            have_min = true;
          } else if (keys.at(i) == "high") {
            param_high = values.at(i).toFloat();
            have_high = true;
          } else if (keys.at(i) == "low") {
            param_low = values.at(i).toFloat();
            have_low = true;
          }
        }
      }
    }
  }

  if (not (have_max and have_min) and hqcApp) {
    QSqlQuery query(hqcApp->systemDB());
    query.exec("SELECT low, high FROM slimits WHERE paramid = ?");
    query.bindValue(0, st.sensor.paramId);
    query.exec();
    if (query.next()) {
      param_min = query.value(0).toFloat();
      param_max = query.value(1).toFloat();
      have_max = have_min = true;
      have_high = have_low = false;
    }
  }

  if ((have_max and value > param_max) or (have_min and value < param_min))
    return OutsideMinMax;

  if ((have_high and value > param_high) or (have_low and value < param_low))
    return OutsideHighLow;

  return Ok;
}

bool KvMetaDataBuffer::isKnownType(int id)
{
  if (not mHaveTypes)
    fetchTypes();

  types_cit it = std::lower_bound(mTypes.begin(), mTypes.end(), id, Helpers::kvTypes_lt_id());
  return (it != mTypes.end() and it->typeID() == id);
}

const kvalobs::kvTypes& KvMetaDataBuffer::findType(int id)
{
  if (not mHaveTypes)
    fetchTypes();

  types_cit it = std::lower_bound(mTypes.begin(), mTypes.end(), id, Helpers::kvTypes_lt_id());
  if (it != mTypes.end() and it->typeID() == id)
    return *it;
  throw std::runtime_error(miutil::StringBuilder() << "type " << id << "' not found");
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
      HQC_LOG_ERROR("exception while retrieving obs_pgm for station " << stationid << ": " << e.what());
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
        HQC_LOG_ERROR("exception retrieving obs_pgm for station list chunk");
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
    if (not KvServiceHelper::instance()->getKvStations(mStations)) {
      HQC_LOG_ERROR("could not fetch station list");
    } else {
      std::vector<kvalobs::kvStation> stations(mStations.begin(), mStations.end());
      std::sort(stations.begin(), stations.end(), Helpers::kvStation_lt());
      mStations = stations_t(stations.begin(), stations.end());
    }
  } catch (std::exception& e) {
    HQC_LOG_ERROR("exception while retrieving station list: " << e.what());
  }
}

void KvMetaDataBuffer::fetchParams()
{
  METLIBS_LOG_SCOPE();
  BusyIndicator wait;
  mHaveParams = true;
  mParams.clear();
  try {
    if (not KvServiceHelper::instance()->getKvParams(mParams)) {
      HQC_LOG_ERROR("could not fetch param list");
    } else {
      std::vector<kvalobs::kvParam> params(mParams.begin(), mParams.end());
      std::sort(params.begin(), params.end(), Helpers::kvParam_lt());
      mParams = params_t(params.begin(), params.end());
    }
  } catch (std::exception& e) {
    HQC_LOG_ERROR("exception while retrieving param list: " << e.what());
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
    if (not KvServiceHelper::instance()->getKvTypes(mTypes)) {
      HQC_LOG_ERROR("could not fetch type list");
    } else {
      std::vector<kvalobs::kvTypes> types(mTypes.begin(), mTypes.end());
      std::sort(types.begin(), types.end(), Helpers::kvTypes_lt());
      mTypes = types_t(types.begin(), types.end());
    }
  } catch (std::exception& e) {
    HQC_LOG_ERROR("exception while retrieving param list: " << e.what());
  }
}

void KvMetaDataBuffer::reload()
{
  mHaveStations = mHaveParams = mHaveTypes = false;
  mObsPgms.clear();
}
