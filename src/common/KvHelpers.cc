
#include "KvHelpers.hh"

#include "HqcApplication.hh"
#include "KvalobsData.hh"
#include "ObsPgmRequest.hh"
#include "TimeSpan.hh"
#include "common/HqcSystemDB.hh"
#include "util/Helpers.hh"

#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvStation.h>

#include <QtCore/QVariant>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

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

  QString ff, sep = QChar(explain ? '\n' : ' ');
  bool first = true;

  const int showFlagAbove[16] = { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0 };
  for(int f=0; f<16; f++) {
    const int flag = cInfo.flag(f);
    using namespace kvalobs::flag;
    if (flag > showFlagAbove[f]) {
      if (not first)
        ff.append(sep);
      ff.append(flagnames[f]);
      ff.append('=');
      ff.append(int2char(flag));
      if (explain) {
        QString explanation = HqcSystemDB::explainFlagValue(f, flag);
        if (explanation.isNull())
          explanation = qApp->translate("Helpers", "Unknown or invalid flag value");
        ff.append(": ");
        ff.append(explanation);
      }
      first = false;
    }
  }
  return ff;
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

kvalobs::kvData getMissingKvData(const SensorTime& st)
{
  const Sensor& s = st.sensor;
  return kvalobs::getMissingKvData(s.stationId, timeutil::to_miTime(st.time),
      s.paramId, s.typeId, s.sensor, s.level);
}

static KvalobsData_p makeData(const SensorTime& st, const timeutil::ptime& tbtime, float original,
    float co, const kvalobs::kvControlInfo& ci, const std::string& cf, bool created)
{
  const Sensor& s = st.sensor;
  kvalobs::kvUseInfo ui;
  ui.setUseFlags(ci);
  const kvalobs::kvData kvdata(s.stationId, st.time, original, s.paramId,
      tbtime, s.typeId, s.sensor, s.level, co, ci, ui, cf);
  return boost::make_shared<KvalobsData>(kvdata, created);
}

KvalobsData_p createdData(const SensorTime& st, const timeutil::ptime& tbtime,
    float co, const kvalobs::kvControlInfo& ci, const std::string& cf)
{
  return makeData(st, tbtime, kvalobs::MISSING, co, ci, cf, true);
}

KvalobsData_p modifiedData(ObsData_p base, float co, const kvalobs::kvControlInfo& ci, const std::string& cf)
{
  return makeData(base->sensorTime(), base->tbtime(), base->original(), co, ci, cf, false);
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

void addNeighbors(Sensor_v& neighbors, const Sensor& sensor, const TimeSpan& time,
    const hqc::kvStation_v& neighborStations, const ObsPgmRequest* obsPgms, int maxNeighbors)
{
  METLIBS_LOG_SCOPE();

  int count = 0;
  BOOST_FOREACH(const kvalobs::kvStation& s, neighborStations) {
    const hqc::kvObsPgm_v& obs_pgm = obsPgms->get(s.stationID());
    BOOST_FOREACH (const kvalobs::kvObsPgm& op, obs_pgm) {
      if (time.intersection(TimeSpan(op.fromtime(), op.totime())).undef())
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

Sensor_v relatedSensors(const Sensor& s, const TimeSpan& time, const std::string& viewType,
    const ObsPgmRequest* obsPgms, const hqc::kvStation_v& neighborStations)
{
  METLIBS_LOG_TIME();

  hqc::int_v stationPar = HqcSystemDB::relatedParameters(s.paramId, QString::fromStdString(viewType));
  if (stationPar.empty())
    stationPar.push_back(s.paramId);

  const int nNeighbors = 8;
  hqc::int_v neighborPar(1, s.paramId);

  const hqc::kvObsPgm_v& obs_pgm = obsPgms->get(s.stationId);
  Sensor_v sensors;
  BOOST_FOREACH(int par, stationPar) {
    Sensor s2(s);
    s2.paramId = par;
    bool accept = (par == s.paramId);
    if (not accept) {
      BOOST_FOREACH (const kvalobs::kvObsPgm& op, obs_pgm) {
        if (time.intersection(TimeSpan(op.fromtime(), op.totime())).undef())
          continue;
        
        const bool eql = op.paramID() == par;
        const bool agg = aggregatedParameter(op.paramID(), par);
        if (eql or agg) {
          accept = true;
          s2.typeId = eql ? op.typeID() : -op.typeID();
          break;
        }
      }
    }
    if (accept)
      sensors.push_back(s2);
  }
    
  BOOST_FOREACH(int par, neighborPar) {
    Sensor sn(s);
    sn.paramId = par;
    addNeighbors(sensors, sn, time, neighborStations, obsPgms, nNeighbors);
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
  HqcSystemDB::aggregatedParameters(paramFrom, paramTo);
}

void updateCfailed(kvalobs::kvData& data, const std::string& add)
{
  std::string new_cfailed = data.cfailed();
  if( new_cfailed.length() > 0 )
    new_cfailed += ",";
  new_cfailed += add;
  data.cfailed(new_cfailed);
}

QString stationName(const kvalobs::kvStation& s)
{
  return QString::fromLatin1(s.name().c_str());
}

QString sensorTimeToString(const SensorTime& st)
{
  const Sensor& s = st.sensor;
  return QString("stationid=%1;level=%2;sensornr=%3;typeid=%4;paramid=%5;time=%6;")
      .arg(s.stationId).arg(s.level).arg(s.sensor).arg(s.typeId).arg(s.paramId)
      .arg(QString::fromStdString(timeutil::to_iso_extended_string(st.time)));
}

SensorTime sensorTimeFromString(const QString& s)
{
  SensorTime st;
  const QStringList parts = s.split(";", QString::SkipEmptyParts);
  for (int i=0; i<parts.size(); ++i) {
    const QString& p = parts.at(i);
    int eq = p.indexOf("=");
    const QString key = p.left(eq);
    const QString value = p.mid(eq+1);
    if (key == "stationid")
      st.sensor.stationId = value.toInt();
    else if (key == "level")
      st.sensor.level = value.toInt();
    else if (key == "sensornr")
      st.sensor.sensor = value.toInt();
    else if (key == "typeid")
      st.sensor.typeId = value.toInt();
    else if (key == "paramid")
      st.sensor.paramId = value.toInt();
    else if (key == "paramid")
      st.sensor.paramId = value.toInt();
    else if (key == "time")
      st.time = timeutil::from_iso_extended_string(value.toStdString());
  }
  return st;
}

} // namespace Helpers
