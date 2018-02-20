
#include "ErrorFilter.hh"

#include "common/Functors.hh"
#include "common/KvHelpers.hh"
#include "common/ObsHelpers.hh"
#include "common/KvMetaDataBuffer.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.ErrorFilter"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {

const int paramid_hour_6_18[] = {
  76,  // DX_12
  80,  // DG_12
  89,  // FX_12
  92,  // FG_12
  109, // RR_12
  214, // TAN_12
  216, // TAX_12
  224  // TGN_12
};

const int paramid_hour_6[] = {
  kvalobs::PARAMID_RR_24,
  kvalobs::PARAMID_SA
};

const int paramid_preciptationtype[] = {
    31, // V1
    32, // V2
    33, // V3
    34, // V4
    35, // V4S
    36, // V5
    37, // V5S
    38, // V6
    39, // V6S
    40, // V7
    41, // WW
    42, // W1
    43  // W2
};

bool IsTypeInObsPgm(int stnr, int par, int typeId, const timeutil::ptime& otime)
{
  const timeutil::pdate otime_date = otime.date();
  
  // this is from HqcMainWindow::checkTypeId combined with from ErrorList::typeFilter
  const hqc::hqcObsPgm_v& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(stnr);
  BOOST_FOREACH(const kvalobs::kvObsPgm& op, obs_pgm) {
    const timeutil::pdate tfrom = timeutil::from_miTime(op.fromtime()).date(),
        tto = timeutil::from_miTime(op.totime()).date();
    if (abs(typeId) == op.typeID() and par == op.paramID() && otime_date >= tfrom && otime_date <= tto )
      return true;
  }
  return false;
}

bool checkErrorSalen2013(const ObsData_p obs)
{
  const kvalobs::kvControlInfo ci = obs->controlinfo();
  const int fhqc = ci.flag(kvalobs::flag::fhqc);
  if (fhqc != 0)
    return false;

  const SensorTime& st = obs->sensorTime();
  const Sensor& sensor = st.sensor;
  if (not Helpers::isNorwegianStationId(sensor.stationId))
    return false;

  const int hour = st.time.time_of_day().hours();

  const bool param_6_18 = std::binary_search(paramid_hour_6_18, boost::end(paramid_hour_6_18), sensor.paramId);
  const bool param_6 = std::binary_search(paramid_hour_6, boost::end(paramid_hour_6), sensor.paramId);
  if ((param_6 and hour != 6) or (param_6_18 and hour != 6 and hour != 18))
    return false;

  if (not IsTypeInObsPgm(sensor.stationId, sensor.paramId, sensor.typeId, st.time))
    return false;

  const int fs = ci.flag(kvalobs::flag::fs);
  const int fr = ci.flag(kvalobs::flag::fr);
  if (fr == 4 || fr == 5 || fr == 6 || fs == 2)
    return true;

  return false;
}

bool checkErrorHQC2013(const ObsData_p obs)
{
  const kvalobs::kvControlInfo ci = obs->controlinfo();
  const int fhqc = ci.flag(kvalobs::flag::fhqc);
  if (fhqc != 0)
    return false;

  const SensorTime& st = obs->sensorTime();
  const Sensor& sensor = st.sensor;
  if (not Helpers::isNorwegianStationId(sensor.stationId))
    return false;

  const int hour = st.time.time_of_day().hours();

  const int fpre = ci.flag(kvalobs::flag::fpre);
  if (fpre == 4 or fpre == 6) {
    const bool error_6_18 = (hour == 6 or hour == 18) and std::binary_search(paramid_hour_6_18, boost::end(paramid_hour_6_18), sensor.paramId);
    const bool error_6 = hour == 6 and std::binary_search(paramid_hour_6, boost::end(paramid_hour_6), sensor.paramId);
    return (error_6 or error_6_18);
  }

  const int ftime = ci.flag(kvalobs::flag::ftime);
  if (ftime > 0)
    return false;

  const int fd = ci.flag(kvalobs::flag::fd);
  if ((fd == 7 or fd == 8) and sensor.paramId == kvalobs::PARAMID_RR_24)
    return false;

  const int fmis = ci.flag(kvalobs::flag::fmis);
  if (fmis != 0) {
    const bool is_precipitationtype = std::binary_search(paramid_preciptationtype, boost::end(paramid_preciptationtype), sensor.paramId);
    if (is_precipitationtype /* && TODO RR > -1 */)
      return false;
  }

  const int ui_2 = Helpers::extract_ui2(obs);
  if (ui_2 == 2 or ui_2 == 3)
    return true;

  const int fr = ci.flag(kvalobs::flag::fr);
  if (fr == 2 or fr == 3)
    return true;

  if (fmis == 3)
    return true;

  return false;
}

bool checkError2013(const ObsData_p obs, bool errorsForSalen)
{
  if (errorsForSalen)
    return checkErrorSalen2013(obs);
  else
    return checkErrorHQC2013(obs);
}

} // namespace anonymous

// ************************************************************************

ErrorFilter::ErrorFilter(bool errorsForSalen)
    : mErrorsForSalen(errorsForSalen)
{
}

ErrorFilter::~ErrorFilter()
{
}

QString ErrorFilter::acceptingSql(const QString& d, const TimeSpan&) const
{
  QString sql;
  sql += "(substr(" + d + "controlinfo,16,1) = '0'"; // fhqc == 0
  if (not mErrorsForSalen) {
    sql += " AND substr(" + d + "controlinfo, 8,1) = '0'" // ftime == 0
        +  " AND NOT (substr(" + d + "controlinfo,13,1) IN ('7','8') AND paramid = 110)" // fd != 7,8 for RR_24
        +  " AND (substr(" + d + "useinfo, 3,1) IN ('2','3')" // useinfo(2) == 2,3
        +  "     OR substr(" + d + "controlinfo,2,1) IN ('2','3')" // fr == 2,3
        +  "     OR substr(" + d + "controlinfo,7,1) = '3')"; // fmis == 3
  } else {
    sql += " AND (substr(" + d + "useinfo, 4,1) = '2'" // fs == 2
        +  "     OR substr(" + d + "controlinfo,2,1) IN ('4','5','6'))"; // fr == 4,5,6
  }
  sql += ")";
  return sql;
}

bool ErrorFilter::accept(ObsData_p obs, bool /*afterSQL*/) const
{
  return checkError2013(obs, mErrorsForSalen);
}

bool ErrorFilter::subsetOf(ObsFilter_p other) const
{
  ErrorFilter_p oe = std::dynamic_pointer_cast<ErrorFilter>(other);
  if (not oe)
    return false;
  if (oe->mErrorsForSalen != this->mErrorsForSalen)
    return false;
  return true;
}
