
#include "KvHelpers.hh"

#include "KvMetaDataBuffer.hh"
#include "TimeRange.hh"
#include "common/HqcApplication.hh"
#include "util/Helpers.hh"
#include "util/stringutil.hh"

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

float numericalValue(int paramId, float codeValue)
{
  if (paramId == kvalobs::PARAMID_RR_24 and codeValue == -1.0)
    return 0.0;
  return codeValue;
}

static const int STATIONID_NORWAY_MIN = 60, STATIONID_NORWAY_MAX = 99999;

bool isNorwegianStationId(int stationid)
{
  return stationid >= STATIONID_NORWAY_MIN and
      stationid <= STATIONID_NORWAY_MAX;
}

std::string isNorwegianStationIdSQL(const std::string& stationid_column)
{
  std::ostringstream sql;
  sql << " (" << stationid_column << " BETWEEN " << STATIONID_NORWAY_MIN
      << " AND " << STATIONID_NORWAY_MAX << ") ";
  return sql.str();
}

int nearestStationId(float lon, float lat, float maxDistanceKm)
{
  int nearestStation = -1;
  float nearestDistance = 0;

  const std::list<kvalobs::kvStation>& stationsList = KvMetaDataBuffer::instance()->allStations();

  try {
    BOOST_FOREACH(const kvalobs::kvStation& s, stationsList) {
      const int sid = s.stationID();
      if (not isNorwegianStationId(sid))
        continue;
      const float d = Helpers::distance(s.lon(), s.lat(), lon, lat);
      if (d > maxDistanceKm)
        continue;
      if (nearestStation < 0 or nearestDistance > d) {
        nearestDistance = d;
        nearestStation = sid;
      }
    }
  } catch (std::exception& e) {
    METLIBS_LOG_WARN("exception while searching nearest station: " << e.what());
    nearestStation = -1;
  }
  return nearestStation;
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
      if (sid == sensor.stationId)
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
        const Sensor n(s.stationID(), sensor.paramId, op.level(), 0, typeId); // TODO also add sensor_nr > 1?
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

typedef std::vector<int> int_v;
typedef std::vector<Sensor> Sensor_v;
typedef std::set<Sensor, lt_Sensor> Sensor_s;

void addSensorsFromObsPgm(Sensor_v& sensors, Sensor_s& selected, const TimeRange& time, int stationId, const int_v& paramIds)
{
  const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(stationId);
  BOOST_FOREACH(int paramId, paramIds) {
    Sensor sensor(stationId, paramId, 0, 0, 0);
    BOOST_FOREACH (const kvalobs::kvObsPgm& op, obs_pgm) {
      const TimeRange op_time(op.fromtime(), op.totime());
      if (time.intersection(op_time).undef())
        continue;

      const int p = op.paramID(), t = op.typeID();
      if (p == paramId)
        sensor.typeId = t;
      else if (aggregatedParameter(op.paramID(), paramId))
        sensor.typeId = -t;
      else
        continue;
#ifdef OBSPGM_IGNORE_NON0_LEVEL
      if (op.level() != 0)
        continue;
#endif
      sensor.level = op.level();
      sensor.sensor = 0;  // TODO also add sensor_nr > 1?
      if (selected.find(sensor) == selected.end()) {
        sensors.push_back(sensor);
        selected.insert(sensor);
        METLIBS_LOG_DEBUG("accept" << LOGVAL(sensor));
      }
    }
  }
}

Sensor_v relatedSensors(const Sensor& s, const TimeRange& time, const std::string& viewType)
{
  METLIBS_LOG_TIME();

  int_v stationPar, neighborPar;
  int nNeighbors = 8;

  if (hqcApp) {
    QSqlQuery query(hqcApp->systemDB());
    query.prepare("SELECT pr1.paramid FROM param_related AS pr1"
        " WHERE pr1.groupid = (SELECT pr2.groupid FROM param_related AS pr2 WHERE pr2.paramid = :pid)"
        "   AND (pr1.view_types_excluded IS NULL OR pr1.view_types_excluded NOT LIKE :vt)"
        " ORDER BY pr1.sortkey");

    query.bindValue(":pid", s.paramId);
    query.bindValue(":vt",  "%" + Helpers::fromUtf8(viewType) + "%");
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

  if (std::find(stationPar.begin(), stationPar.end(), s.paramId) == stationPar.end())
    stationPar.insert(stationPar.begin(), s.paramId);
  if (neighborPar.empty())
    neighborPar.push_back(s.paramId);

  Sensor_v sensors;
  Sensor_s selected;
  addSensorsFromObsPgm(sensors, selected, time, s.stationId, stationPar);

  BOOST_FOREACH(int par, neighborPar) {
    Sensor sn(s);
    sn.paramId = par;
    addNeighbors(sensors, sn, time, nNeighbors);
  }
#if 1
  METLIBS_LOG_DEBUG("found " << sensors.size() << " related sensors for " << LOGVAL(s));
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

QString paramName(int paramId)
{
  return Helpers::fromUtf8(KvMetaDataBuffer::instance()->findParamName(paramId));
}

QString stationName(const kvalobs::kvStation& s)
{
  return QString::fromLatin1(s.name().c_str());
}

QString paramInfo(int paramId)
{
  QString info = QString::number(paramId) + ": ";
  try {
    const kvalobs::kvParam& p = KvMetaDataBuffer::instance()->findParam(paramId);
    info += Helpers::fromUtf8(p.description());
  } catch (std::exception& e) {
    info += "?";
  }
  return info;
}

QString typeInfo(int typeId)
{
  QString info = QString::number(typeId) + ": ";
  try {
    const kvalobs::kvTypes& t = KvMetaDataBuffer::instance()->findType(typeId);
    info += Helpers::fromUtf8(t.format());
  } catch (std::exception& e) {
    info += "?";
  }
  if (typeId < 0)
    info += qApp->translate("Helpers", " generated by kvalobs");
  return info;
}

QString stationInfo(int stationId)
{
  try {
    const kvalobs::kvStation& s = KvMetaDataBuffer::instance()->findStation(stationId);
    return QString(qApp->translate("Helpers", "%1 %2 %3masl."))
        .arg(stationId).arg(Helpers::fromUtf8(s.name())).arg(s.height());
  } catch (std::exception& e) {
    return QString::number(stationId);
  }
}

} // namespace Helpers
