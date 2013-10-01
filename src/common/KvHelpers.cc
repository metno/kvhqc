
#include "KvHelpers.hh"

#include "KvMetaDataBuffer.hh"
#include "TimeRange.hh"
#include "common/gui/HqcApplication.hh"
#include "util/Helpers.hh"

#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvStation.h>

#include <QtCore/QVariant>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvHelpers"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {
const char* flagnames[16] = {
    "fagg", "fr", "fcc", "fs", "fnum", 
    "fpos", "fmis", "ftime", "fw", "fstat", 
    "fcp", "fclim", "fd", "fpre", "fcombi", "fhqc" 
};
} // namespace anonymous

// ########################################################################

namespace Helpers {

bool stations_by_distance::operator()(const kvalobs::kvStation& a, const kvalobs::kvStation& b) const
{
  if (a.stationID() == b.stationID())
    return false;
  return distance(a) < distance(b);
}

// ------------------------------------------------------------------------

float stations_by_distance::distance(const kvalobs::kvStation& neighbor) const
{
  return Helpers::distance(center.lon(), center.lat(), neighbor.lon(), neighbor.lat());
}

// ########################################################################

bool stations_by_id::operator()(const kvalobs::kvStation& a) const
{
  return (a.stationID() == stationid);
}

// ########################################################################

int is_accumulation(const kvalobs::kvControlInfo& ci)
{
    using namespace kvalobs::flag;
    const int f_fd = ci.flag(fd);
    if( f_fd == 2 or f_fd == 4 )
        return BEFORE_REDIST;
    if( f_fd == 7 or f_fd == 8 )
        return QC2_REDIST;
    if( f_fd == 9 or f_fd == 0xA )
        return HQC_REDIST;
    return NO;
}

// ------------------------------------------------------------------------

int is_endpoint(const kvalobs::kvControlInfo& ci)
{
    using namespace kvalobs::flag;
    const int f_fd = ci.flag(fd);
    if( f_fd == 4 )
        return BEFORE_REDIST;
    if( f_fd == 8 )
        return QC2_REDIST;
    if( f_fd == 0xA )
        return HQC_REDIST;
    return NO;
}

// ------------------------------------------------------------------------

bool is_rejected(const kvalobs::kvControlInfo& ci, float corr)
{
    return (ci.flag(kvalobs::flag::fmis) == 2) // same as kvDataOperations.cc
        or (corr == kvalobs::REJECTED);
}

// ------------------------------------------------------------------------

bool is_missing(const kvalobs::kvControlInfo& ci, float corr)
{
    return (ci.flag(kvalobs::flag::fmis) == 3) // same as kvDataOperations.cc
        or (corr == kvalobs::MISSING);
}

// ------------------------------------------------------------------------

bool is_orig_missing(const kvalobs::kvControlInfo& ci, float orig)
{
    return (ci.flag(kvalobs::flag::fmis) & 1) // same as kvDataOperations.cc
        or (orig == kvalobs::MISSING);
}

// ------------------------------------------------------------------------

static QString formatFlag(const kvalobs::kvControlInfo & cInfo, bool explain)
{
  METLIBS_LOG_TIME();
  std::auto_ptr<QSqlQuery> query;
  if (explain and hqcApp) {
    query.reset(new QSqlQuery(hqcApp->systemDB()));
    query->prepare("SELECT description FROM flag_explain WHERE flag = :fn AND flagvalue = :fv AND language = 'nb'");
  }

    std::ostringstream ss;
    bool first = true;

    const int showFlagAbove[16] = { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0 };
    for(int f=0; f<16; f++) {
	const int flag = cInfo.flag(f);
        using namespace kvalobs::flag;
	if (flag > showFlagAbove[f]) {
            if( not first )
                ss << (query.get() ? '\n' : ' ');
            ss << flagnames[f] << '=' << int2char(flag);
            if (query.get()) {
              query->bindValue("fn", f);
              query->bindValue("fv", flag);
              query->exec();
              QString explanation;
              if (query->next())
                explanation = query->value(0).toString();
              else
                explanation = qApp->translate("Helpers", "Unknown or invalid flag value");
              query->finish();
              ss << ": " << explanation.toStdString();
            }
            first = false;
	}
    }
    return QString::fromStdString(ss.str());
}


QString getFlagText(const kvalobs::kvControlInfo & cInfo)
{
    return formatFlag(cInfo, false);
}

QString getFlagExplanation(const kvalobs::kvControlInfo & cInfo)
{
    return formatFlag(cInfo, true);
}

QString getFlagName(int flagNumber)
{
    if (flagNumber < 0 or flagNumber >= 16)
        return "";
    return flagnames[flagNumber];
}

QString parameterName(int paramId)
{
    return QString::fromStdString(KvMetaDataBuffer::instance()->findParamName(paramId));
}

int parameterIdByName(const std::string& paramName)
{
  try {
    const std::list<kvalobs::kvParam>& allParams = KvMetaDataBuffer::instance()->allParams();
    BOOST_FOREACH(const kvalobs::kvParam& p, allParams) {
      if (p.name() == paramName)
        return p.paramID();
    }
  } catch (std::exception& e) {
    HQC_LOG_ERROR("exception while fetching params: " << e.what());
  }
  return -1;
}

void updateUseInfo(kvalobs::kvData& data)
{
    kvalobs::kvUseInfo ui = data.useinfo();
    ui.setUseFlags( data.controlinfo() );
    data.useinfo( ui );
}

QString stationName(const kvalobs::kvStation& s)
{
  return QString::fromLatin1(s.name().c_str());
}

float numericalValue(int paramId, float codeValue)
{
  if (paramId == kvalobs::PARAMID_RR_24 and codeValue == -1.0)
    return 0.0;
  return codeValue;
}

void addNeighbors(std::vector<Sensor>& neighbors, const Sensor& sensor, const TimeRange& time, int maxNeighbors)
{
  METLIBS_LOG_SCOPE();

  if (not hqcApp) {
    METLIBS_LOG_INFO("no KvApp -- no neighbors, probably running a test program");
    return;
  }
  
  const std::list<kvalobs::kvStation>& stationsList = KvMetaDataBuffer::instance()->allStations();
  
  std::vector<kvalobs::kvStation> stations;
  try {
    Helpers::stations_by_distance ordering(KvMetaDataBuffer::instance()->findStation(sensor.stationId));
    
    BOOST_FOREACH(const kvalobs::kvStation& s, stationsList) {
      const int sid = s.stationID();
      if (sid < 60 or sid >= 100000 or sid == sensor.stationId)
        continue;
      if (ordering.distance(s) > 100 /*km*/)
        continue;
      stations.push_back(s);
      METLIBS_LOG_DEBUG(LOGVAL(s.stationID()));
    }
    std::sort(stations.begin(), stations.end(), ordering);
  } catch (std::exception& e) {
    METLIBS_LOG_WARN("exception while searching neighbor stations: " << e.what());
    return;
  }
  
  int count = 0;
  BOOST_FOREACH(const kvalobs::kvStation& s, stations) {
    const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(s.stationID());
    BOOST_FOREACH (const kvalobs::kvObsPgm& op, obs_pgm) {
      if (time.intersection(TimeRange(op.fromtime(), op.totime())).undef())
        continue;
      // FIXME this is not correct if there is more than one klXX or collector or typeid or ...
      
      const bool eql = op.paramID() == sensor.paramId;
      const bool agg = aggregatedParameter(op.paramID(), sensor.paramId);
      if (eql or agg) {
        const int typeId = eql ? op.typeID() : -op.typeID();
        const Sensor n(s.stationID(), sensor.paramId, op.level(), 0, typeId);
        const bool duplicate = (std::find_if(neighbors.begin(), neighbors.end(), std::bind1st(eq_Sensor(), n)) != neighbors.end());
        METLIBS_LOG_DEBUG(LOGVAL(n) << LOGVAL(duplicate));
        neighbors.push_back(n);
        if (++count >= maxNeighbors)
          break;
      }
    }
    if (count >= maxNeighbors)
      break;
  }
}

typedef std::vector<Sensor> Sensors_t;
Sensors_t relatedSensors(const Sensor& s, const TimeRange& time, const std::string& viewType)
{
  METLIBS_LOG_TIME();

  std::vector<int> stationPar, neighborPar;
  int nNeighbors = 8;

  if (hqcApp) {
    QSqlQuery query(hqcApp->systemDB());
    query.prepare("SELECT pr1.paramid FROM param_related AS pr1"
        " WHERE pr1.groupid = (SELECT pr2.groupid FROM param_related AS pr2 WHERE pr2.paramid = :pid)"
        "   AND (pr1.view_types_excluded IS NULL OR pr1.view_types_excluded NOT LIKE :vt)"
        " ORDER BY pr1.sortkey");

    query.bindValue(":pid", s.paramId);
    query.bindValue(":vt",  "%" + QString::fromStdString(viewType) + "%");
    query.exec();
    while (query.next())
      stationPar.push_back(query.value(0).toInt());
  }
#if 1
  METLIBS_LOG_DEBUG("found " << stationPar.size() << " station pars for " << LOGVAL(s) << LOGVAL(time) << LOGVAL(viewType));
  BOOST_FOREACH(int pid, stationPar) {
    METLIBS_LOG_DEBUG(LOGVAL(pid));
  }
#endif

  if (stationPar.empty())
    stationPar.push_back(s.paramId);
  if (neighborPar.empty())
    neighborPar.push_back(s.paramId);

  const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(s.stationId);
  Sensors_t sensors;
  Sensor s2(s);
  BOOST_FOREACH(int par, stationPar) {
    BOOST_FOREACH (const kvalobs::kvObsPgm& op, obs_pgm) {
      if (time.intersection(TimeRange(op.fromtime(), op.totime())).undef())
        continue;
      
      const bool eql = op.paramID() == par;
      const bool agg = aggregatedParameter(op.paramID(), par);
      if (eql or agg) {
        s2.paramId = par;
        s2.typeId = eql ? op.typeID() : -op.typeID();
        sensors.push_back(s2);
        break;
      }
    }
  }
    
  BOOST_FOREACH(int par, neighborPar) {
    Sensor sn(s);
    sn.paramId = par;
    addNeighbors(sensors, sn, time, nNeighbors);
  }
#if 0
  METLIBS_LOG_DEBUG("found " << sensors.size() << " default sensors for " << LOGVAL(st) << LOGVAL(viewType));
  BOOST_FOREACH(const Sensor& ds, sensors) {
    METLIBS_LOG_DEBUG(LOGVAL(ds));
  }
#endif
  return sensors;
}

bool aggregatedParameter(int paramFrom, int paramTo)
{
  std::set<int> pTo;
  aggregatedParameters(paramFrom, pTo);
  return pTo.find(paramTo) != pTo.end();
}

void aggregatedParameters(int paramFrom, std::set<int>& paramTo)
{
  METLIBS_LOG_SCOPE();
  if (hqcApp) {
    QSqlQuery query(hqcApp->systemDB());
    query.prepare("SELECT paramid_to FROM param_aggregated WHERE paramid_from = ?");
    query.bindValue(0, paramFrom);
    if (query.exec()) {
      while (query.next())
        paramTo.insert(query.value(0).toInt());
    } else {
      HQC_LOG_WARN("error getting aggregated parameters for " << paramFrom
          << ": " << query.lastError().text());
    }
  }
}

void updateCfailed(kvalobs::kvData& data, const std::string& add)
{
    std::string new_cfailed = data.cfailed();
    if( new_cfailed.length() > 0 )
        new_cfailed += ",";
    new_cfailed += add;
    data.cfailed(new_cfailed);
}

QString typeInfo(int typeID)
{
    try {
        const kvalobs::kvTypes& t = KvMetaDataBuffer::instance()->findType(abs(typeID));

        std::vector<std::string> formats;
        boost::split(formats, t.format(), boost::is_any_of(" ,"));
        if (formats.empty())
            return QString::number(typeID);

        QString info = qApp->translate("Helpers", "%1-station").arg(QString::fromStdString(formats[0]));
        if (typeID < 0)
            info += qApp->translate("Helpers", " generated by kvalobs");

        return info;
    } catch (std::exception&) {
        return QString::number(typeID);
    }
}

QString stationInfo(int stationID)
{
    try {
        const kvalobs::kvStation& s = KvMetaDataBuffer::instance()->findStation(stationID);
        return QString(qApp->translate("Helpers", "%1 %2 %3masl."))
            .arg(stationID).arg(QString::fromStdString(s.name())).arg(s.height());
    } catch (std::exception& e) {
        return QString::number(stationID);
    }
}

} // namespace Helpers
