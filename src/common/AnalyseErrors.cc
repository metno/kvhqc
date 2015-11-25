
#include "AnalyseErrors.hh"

#include "ObsHelpers.hh"
#include "KvMetaDataBuffer.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.AnalyseErrors"
#include "util/HqcLogging.hh"

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

bool IsTypeInObsPgm(int stnr, int par, int typeId, const timeutil::ptime& otime)
{
    const timeutil::pdate otime_date = otime.date();

    // this is from HqcMainWindow::checkTypeId combined with from ErrorList::typeFilter
    const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(stnr);
    BOOST_FOREACH(const kvalobs::kvObsPgm& op, obs_pgm) {
        const timeutil::pdate tfrom = timeutil::from_miTime(op.fromtime()).date(),
            tto = timeutil::from_miTime(op.totime()).date();
        if (abs(typeId) == op.typeID() and par == op.paramID() && otime_date >= tfrom && otime_date <= tto )
            return true;
    }
    return false;
}

bool checkErrorSalen2013(const EditDataPtr obs)
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

bool checkErrorHQC2013(const EditDataPtr obs)
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

  const int ui_2 = Helpers::extract_ui2(obs);
  if (ui_2 == 2)
    return true;

  const int fr = ci.flag(kvalobs::flag::fr);
  if (ui_2 == 3)
    return (fr != 0xA);
  if (fr == 2 or fr == 3)
    return true;

  const int fmis = ci.flag(kvalobs::flag::fmis);
  if (fmis == 3)
    return true;

  return false;
}

bool checkError2013(const EditDataPtr obs, bool errorsForSalen)
{
  if (errorsForSalen)
    return checkErrorSalen2013(obs);
  else
    return checkErrorHQC2013(obs);
}

} // namespace anonymous

// ************************************************************************

namespace Errors {

bool recheck(ErrorInfo& ei, bool errorsForSalen)
{
    const bool oldBad = ei.badInList;
    ei.badInList = 0;
    if (checkError2013(ei.obs, errorsForSalen))
      ei.badInList |= ErrorInfo::BAD_IN_ERRORLIST2013;
    return (ei.badInList != oldBad);
}

Errors_t fillMemoryStore2(EditAccessPtr eda, const Sensors_t& sensors, const TimeRange& limits, bool errorsForSalen)
{
    // FIXME add timeFilter from ClockDialog
    METLIBS_LOG_SCOPE();

    Errors_t memStore2;
    const ObsAccess::DataSet allData = eda->allData(sensors, limits);
    BOOST_FOREACH(const ObsDataPtr& obs, allData) {
      EditDataPtr ebs = boost::static_pointer_cast<EditData>(obs);
      ErrorInfo ei(ebs);
      recheck(ei, errorsForSalen);
      if (ei.badInList != 0)
        memStore2.push_back(ei);
    }

    return memStore2;
}

} // namespace Errors
