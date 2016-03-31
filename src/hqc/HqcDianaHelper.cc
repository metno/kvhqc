
#include "HqcDianaHelper.hh"

#include "common/KvMetaDataBuffer.hh"
#include "common/ModelData.hh"
#include "util/stringutil.hh"

#include <puTools/miStringFunctions.h>
#include <coserver/ClientButton.h>
#include <coserver/miMessage.h>
#include <coserver/QLetterCommands.h>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <sstream>

#define M_TIME
#define MILOGGER_CATEGORY "kvhqc.HqcDianaHelper"
#include "common/ObsLogging.hh"

// temporarily disable sending any data
#define DO_NOT_SEND_ANY_DATA

namespace /* anonymous */ {

const size_t MAX_TIME_CONFIRMATION_TESTS = 2;

/*
  Convert to "Diana-value" of range check flag
*/
int numCode1(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib == 2 || nib == 3 )
    code = 2;
  else if ( nib == 4 || nib == 5 )
    code = 3;
  else if ( nib == 6 )
    code = 9;
  return code;
}

/*
  Convert to "Diana-value" of consistency check flag
*/
int numCode2(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib >= 2 && nib <= 7 )
    code = 8;
  else if ( nib == 10 || nib == 11 )
    code = 7;
  else if ( nib == 13 )
    code = 9;
  return code;
}

/*
  Convert to "Diana-value" of prognostic space control flag
*/
int numCode3(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib == 2 || nib == 3 )
    code = 2;
  else if ( nib == 4 || nib == 5 )
    code = 3;
  else if ( nib == 6 )
    code = 5;
  return code;
}

/*
  Convert to "Diana-value" of step check flag
*/
int numCode4(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib == 2 || nib == 3 )
    code = 2;
  else if ( nib >= 4 && nib <= 7 )
    code = 8;
  else if ( nib == 9 || nib == 10 )
    code = 7;
  return code;
}

/*
  Convert to "Diana-value" of timeseries adaption flag
*/
int numCode5(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 || nib == 2 )
    code = 5;
  else if ( nib == 3 )
    code = 3;
  return code;
}

/*
  Convert to "Diana-value" of statistics control flag
*/
int numCode6(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib == 2 )
    code = 3;
  return code;
}

/*
  Convert to "Diana-value" of climatology control flag
*/
int numCode7(int nib) {
  int code = 9;
  if ( nib == 0 )
    code = 0;
  else if ( nib == 1 )
    code = 1;
  else if ( nib == 2 )
    code = 3;
  else if ( nib == 3 )
    code = 7;
  return code;
}

/*
  Convert to "Diana-value" of HQC flag
*/
int numCode8(int nib) {
  int code = 9;
  if ( nib <  10 )
    code = nib;
  else
    code = 9;
  return code;
}

// was: int KvalobsData::flag(std::size_t parameter)
int flag4obs(ObsDataPtr obs)
{
  if (not obs)
    return 0;
  const kvalobs::kvControlInfo controlInfo = obs->controlinfo();

  // Find flags from the different checks

  int nib1  =controlInfo.flag(1);
  int nib2  =controlInfo.flag(2);
  int nib3  =controlInfo.flag(4);
  int nib4  =controlInfo.flag(3);
  int nib5  =controlInfo.flag(7);
  int nib6  =controlInfo.flag(9);
  int nib7  =controlInfo.flag(11);
  int nib8  =controlInfo.flag(10);
  int nib9  =controlInfo.flag(12);
  int nib10 =controlInfo.flag(15);
  // Decode flags

  int nc1 = numCode1(nib1); // Range check
  int nc2 = numCode2(nib2); // Formal Consistency check
  int nc8 = numCode2(nib8); // Climatologic Consistency check
  // Use the largest value from these checks
  nc1 = nc1 > nc2 ? nc1 : nc2;
  nc1 = nc1 > nc8 ? nc1 : nc8;
  nc2 = numCode3(nib3); //Prognostic space control
  int nc3 = numCode4(nib4); //Step check
  int nc4 = numCode5(nib5); //Timeseries adaption
  int nc5 = numCode6(nib6); //Statistics control
  int nc6 = numCode7(nib7); //Climatology control
  // Use the largest value from the three last checks
  nc4 = nc4 > nc5 ? nc4 : nc5;
  nc4 = nc4 > nc6 ? nc4 : nc6;
  if ( nib9 > 1 )
    nc4 = nc4 > 6 ? nc4 : 6;
  nc5 = numCode8(nib10);

  int nc = 10000*nc1 + 1000*nc2 + 100*nc3 + 10*nc4 + nc5;
  return nc;
}

} // anonymous namespace

HqcDianaHelper::HqcDianaHelper(ClientButton* pluginB)
  : mClientButton(pluginB)
  , mDianaConnected(false)
  , mDianaNeedsHqcInit(true)
  , mDianaSensorTime(Sensor(0,0,0,0,0), timeutil::ptime())
  , mCountSameTime(0)
  , mEnabled(true)
{
  mClientButton->useLabel(true);
  mClientButton->connectToServer();

  connect(mClientButton, SIGNAL(receivedMessage(const miMessage&)), SLOT(processLetter(const miMessage&)));
  connect(mClientButton, SIGNAL(addressListChanged()), this, SLOT(handleAddressListChanged()));
  connect(mClientButton, SIGNAL(connectionClosed()),   this, SLOT(handleConnectionClosed()));

  updateDianaParameters();
}

void HqcDianaHelper::setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits)
{
  METLIBS_LOG_SCOPE(LOGVAL(limits));
  DataView::setSensorsAndTimes(sensors, limits);

  mDianaNeedsHqcInit = true;
  mSensors = sensors;
  mTimeLimits = limits;
  mDianaSensorTime.sensor = mSensors.empty() ? Sensor(0,0,0,0,0) : mSensors.front();

  ObsAccess::TimeSet allTimes;
  BOOST_FOREACH(const Sensor& s, sensors) {
    METLIBS_LOG_DEBUG(LOGVAL(s));
    if (s.stationId > 0)
      mDA->addAllTimes(allTimes, s, limits);
    else
      HQC_LOG_WARN("stationId = 0: " << s);
  }
  sendTimes(allTimes);
}

void HqcDianaHelper::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr data)
{
  METLIBS_LOG_SCOPE();

  if (not data or not eq_SensorTime()(mDianaSensorTime, data->sensorTime()))
    return;

  sendObservations();
}

void HqcDianaHelper::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_TIME();

  if (eq_SensorTime()(mDianaSensorTime, st))
    return;

  mDianaSensorTime = st;
  METLIBS_LOG_DEBUG(LOGVAL(mDianaSensorTime));

  sendTime();
  sendObservations();
  sendStation();
  sendSelectedParam();
}

void HqcDianaHelper::setEnabled(bool enabled)
{
  if (enabled == mEnabled)
    return;

  mEnabled = enabled;
  if (mEnabled and mDianaConnected) {
    mDianaNeedsHqcInit = true;
    mTimesAwaitingConfirmation.clear();
    sendTimes();
    sendTime();
  }
}

void HqcDianaHelper::handleAddressListChanged()
{
  METLIBS_LOG_SCOPE();
  const bool dc = (mClientButton->clientTypeExist("Diana"));
  METLIBS_LOG_DEBUG(LOGVAL(dc) << LOGVAL(mDianaConnected));
  if (dc == mDianaConnected)
    return;

  mDianaConnected = dc;
  mTimesAwaitingConfirmation.clear();
  if (mDianaConnected) {
    mDianaNeedsHqcInit = true;
    sendTimes();
    sendTime();
  }
  Q_EMIT connectedToDiana(mDianaConnected);
}

void HqcDianaHelper::handleConnectionClosed()
{
  METLIBS_LOG_SCOPE();
  mDianaConnected = false;
  mTimesAwaitingConfirmation.clear();
  Q_EMIT connectedToDiana(false);
}

void HqcDianaHelper::sendTimes(const std::set<timeutil::ptime>& allTimes)
{
  METLIBS_LOG_SCOPE();

  mAllTimes = allTimes;
  sendTimes();
  if (mEnabled and not isKnownTime(mDianaSensorTime.time)) {
    METLIBS_LOG_DEBUG(LOGVAL(mDianaSensorTime));
    if (not mAllTimes.empty()) {
      mDianaSensorTime.time = *mAllTimes.begin();
      METLIBS_LOG_DEBUG(LOGVAL(mDianaSensorTime));
      sendTime();
    } else {
      mDianaSensorTime.time = timeutil::ptime();
      METLIBS_LOG_DEBUG(LOGVAL(mDianaSensorTime));
    }
  }
}

void HqcDianaHelper::sendTimes()
{
  METLIBS_LOG_SCOPE(LOGVAL(mEnabled) << LOGVAL(mDianaConnected) << LOGVAL(mAllTimes.size()));
  if (not mEnabled or not mDianaConnected or mAllTimes.empty())
    return;

  miMessage m;
  m.command= qmstrings::settime;
  m.commondesc = "datatype";
  m.common = "obs";
  m.description= "time";
  BOOST_FOREACH(const timeutil::ptime& t, mAllTimes)
      m.data.push_back(timeutil::to_iso_extended_string(t));
  sendMessage(m);
}

void HqcDianaHelper::processLetter(const miMessage& letter)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(letter.content()));
  if (letter.command == qmstrings::newclient) {
    std::vector<std::string> desc, valu;
    const std::string cd(letter.commondesc), co(letter.common);
    boost::split(desc, cd, boost::is_any_of(":"));
    boost::split(valu, co, boost::is_any_of(":"));
    for(std::vector<std::string>::const_iterator itD=desc.begin(), itC=valu.begin(); itD != desc.end(); ++itC, ++itD) {
      if( *itD == "type" && *itC == "Diana" ) {
        handleAddressListChanged();
        return;
      }
    }
  } else if (letter.command == qmstrings::station and mEnabled) {
    if (letter.commondesc == "name,time") {
      QStringList name_time = Helpers::fromUtf8(letter.common).split(',');
      bool ok = false;
      const int stationid = name_time[0].mid(1).toInt(&ok);
      if (not ok) {
        METLIBS_LOG_INFO("Unable to parse first element as stationid: '" << letter.common << "'");
        return;
      }
      handleDianaStationAndTime(stationid, name_time[1].toStdString());
    }
  } else if (letter.command == qmstrings::timechanged and mEnabled) {
    handleDianaStationAndTime(0, letter.common);
  } else if (letter.command == qmstrings::positions and mEnabled) {
    if (letter.commondesc == "dataset" and letter.data.size() == 1) {
      typedef std::vector<std::string> string_v;
      string_v description, data;
      boost::split(description, letter.description,  boost::is_any_of(":"));
      boost::split(data,        letter.data.front(), boost::is_any_of(":"));

      const int idx_lat = std::find(description.begin(), description.end(), "lat") - description.begin();
      const int idx_lon = std::find(description.begin(), description.end(), "lon") - description.begin();
      if (description.size() == 2 and idx_lon >= 0 and idx_lat >= 0) {
        const float lon = miutil::to_float(data[idx_lon]);
        const float lat = miutil::to_float(data[idx_lat]);
        handlePosition(lon, lat);
        return;
      }
    }
    METLIBS_LOG_INFO("Unable to handle positions message: '"
        << "====================\n" << letter.content() << "====================");
  } else if (letter.command != qmstrings::registeredclient && letter.command != qmstrings::unregisteredclient) {
    METLIBS_LOG_INFO("Diana sent a command that HQC does not understand:\n"
        << "====================\n" << letter.content() << "====================");
  }
}

bool HqcDianaHelper::switchToKvalobsStationId(int stationId)
{
  if (stationId == 0 or stationId == mDianaSensorTime.sensor.stationId)
    return false;

  BOOST_FOREACH(const Sensor& s, mSensors) {
    // TODO how to identify the correct sensor?
    if (s.stationId == stationId and s.paramId == mDianaSensorTime.sensor.paramId) {
      mDianaSensorTime.sensor = s;
      return true;
    }
  }
  return false;
}

void HqcDianaHelper::handleDianaStationAndTime(int stationId, const std::string& time_txt)
{
  if (not mEnabled)
    return;
  METLIBS_LOG_SCOPE();

  bool sendSignal = false;

  const timeutil::ptime time = timeutil::from_iso_extended_string(time_txt);
  if (not isKnownTime(time)) {
    METLIBS_LOG_INFO("Invalid/unknown time from diana: '" << time_txt << "'");
    mDianaNeedsHqcInit = true;
    sendTimes();
    sendTime();
  } else {
    const TimesAwaitingConfirmation_t::iterator itB = mTimesAwaitingConfirmation.begin(),
        itE = mTimesAwaitingConfirmation.end();
    TimesAwaitingConfirmation_t::iterator itS = itB;
    bool foundTC = false;
    for (size_t count = 0; not foundTC and count < MAX_TIME_CONFIRMATION_TESTS and itS != itE; ++count) {
      METLIBS_LOG_DEBUG("comparing time '" << time << "' with confirmation time[" << count << "]=" << *itS);
      if (*itS == time)
        foundTC = true;
      // always advance iterator, we use it in erase later
      ++itS;
    }
    mDianaSensorTime.time = time;
    if (foundTC) {
      METLIBS_LOG_DEBUG("time '" << time << "' found in confirmation list");
      mTimesAwaitingConfirmation.erase(itB, itS);
    } else {
      METLIBS_LOG_DEBUG("time '" << time << "' not found in confirmation list, assuming it is from a click in diana");
      mTimesAwaitingConfirmation.clear();

      METLIBS_LOG_DEBUG("re-sending observations" << LOGVAL(mDianaSensorTime));
      sendObservations();
      sendSignal = true;
    }
  }
  sendSignal |= switchToKvalobsStationId(stationId);

  if (sendSignal) {
    METLIBS_LOG_DEBUG(LOGVAL(mDianaSensorTime));
    Q_EMIT signalNavigateTo(mDianaSensorTime);
  }
}

void HqcDianaHelper::handlePosition(float lon, float lat)
{
  METLIBS_LOG_SCOPE();
  if (not mEnabled)
    return;
  const int nearest = Helpers::nearestStationId(lon, lat);
  METLIBS_LOG_DEBUG(LOGVAL(nearest) << LOGVAL(lon) << LOGVAL(lat));
  if (nearest <= 0)
    return;

  if (switchToKvalobsStationId(nearest)) {
    METLIBS_LOG_DEBUG(LOGVAL(mDianaSensorTime));
    Q_EMIT signalNavigateTo(mDianaSensorTime);
  }
}

void HqcDianaHelper::applyQuickMenu()
{
  METLIBS_LOG_SCOPE();
  if (not mEnabled)
    return;
#if 0 // FIXME this does not actually show the data
  miMessage m;
  m.command = qmstrings::apply_quickmenu;
  m.data.push_back("hqc_qmenu");
  m.data.push_back("hqc_obs_...");
  sendMessage(m);
#endif
}

void HqcDianaHelper::sendStation()
{
#ifndef DO_NOT_SEND_ANY_DATA
  METLIBS_LOG_SCOPE();
  if (not mEnabled or not mDianaConnected)
    return;
  miMessage m;
  m.command = qmstrings::station;
  m.common  = "S" + boost::lexical_cast<std::string>(mDianaSensorTime.sensor.stationId);
  sendMessage(m);
#endif // DO_NOT_SEND_ANY_DATA
}

void HqcDianaHelper::sendTime()
{
  METLIBS_LOG_SCOPE();
  if (not mEnabled or not mDianaConnected or not isKnownTime(mDianaSensorTime.time))
    return;

  mTimesAwaitingConfirmation.push_back(mDianaSensorTime.time);
  METLIBS_LOG_DEBUG(LOGVAL(mDianaSensorTime.time) << LOGVAL(mTimesAwaitingConfirmation.size()));

  miMessage m;
  m.command = qmstrings::settime;
  m.commondesc = "time";
  m.common = timeutil::to_iso_extended_string(mDianaSensorTime.time);
  sendMessage(m);
}

bool HqcDianaHelper::isKnownTime(const timeutil::ptime& time) const
{
  return (not time.is_not_a_date_time())
      and (mAllTimes.find(time) != mAllTimes.end());
}

std::string HqcDianaHelper::synopStart(int stationId)
{
  std::ostringstream synop;
  try {
    const kvalobs::kvStation& station = KvMetaDataBuffer::instance()->findStation(stationId);

    synop << "S" << stationId << ',';

    const std::string stationType = hqcType(stationId, station.environmentid());
    if (stationType == "AA" or stationType == "VM" or stationType == "none")
      synop << "none";
    else if (stationType.at(0) == 'P')
      synop << 'P';
    else
      synop << stationType.at(1);
    synop << ',';

    std::string isAuto = "x";
    if (stationType.at(0) == 'A')
      isAuto = "a";
    else if (stationType.at(0) == 'N' || stationType.at(0) == 'P')
      isAuto = "n";
    synop << isAuto << ',';

    synop << (boost::format("%1$.4f,%2$.4f") % station.lon() % station.lat()).str();
  } catch (std::exception&) {
    // ignore unknown station
  }
  return synop.str();
}

std::string HqcDianaHelper::synopValue(const SensorTime& st, const SendPar& sp, bool& hasData)
{
  double corr = -32767;

  ObsDataPtr obs = mDA->find(st);
  if (obs) {
    corr = obs->corrected();
    if (sp.what != SendPar::CORRECTED) {
      ModelDataPtr mdl = mMA->find(st);
      if (mdl) {
        const double modelValue = mdl->value();
        if (sp.what == SendPar::MODEL)
          corr = modelValue;
        else if (sp.what == SendPar::DIFF)
          corr = corr - modelValue;
        else if (sp.what == SendPar::PROPORTION)
          corr = (abs(modelValue) > 0.00001) ? corr/modelValue : -32767;
      } else {
        corr = -32767;
      }
    }
  }
  if (corr < -999)
    return ",-32767";

  double corrAA = -32767;
  {
    SensorTime stAA(st);
    stAA.sensor.paramId = 1 /* AA */;
    ObsDataPtr obsAA = mDA->find(stAA);
    if (obsAA)
      corrAA = obsAA->corrected();
  }

  corr = dianaValue(st.sensor.paramId, sp.what == SendPar::MODEL, corr, corrAA);
  const int flag = flag4obs(obs);
  const int shFl1 = flag/10000;
  const int shFl2 = flag%10000/1000;
  const int shFl3 = flag%1000/100;
  const int shFl4 = flag%100/10;
  const int shFl5 = flag%10;
  int maxFlag = shFl1 >shFl2 ? shFl1 : shFl2;
  maxFlag = shFl3 > maxFlag ? shFl3 : maxFlag;
  maxFlag = shFl4 > maxFlag ? shFl4 : maxFlag;
  const std::string flagstr = (boost::format("%1$05d") % flag).str();
  std::string colorstr;
  if (maxFlag == 0)
    colorstr = ";0:0:0";
  if (maxFlag == 1 or (shFl5 != 0 && shFl5 != 9))
    colorstr = ";0:255:0";
  else if (maxFlag >= 2 && maxFlag <= 5)
    colorstr = ";255:175:0";
  else if (maxFlag >= 6)
    colorstr = ";255:0:0";

  hasData = true;
  std::ostringstream synop;
  synop << ',' << corr << ';' << flagstr << colorstr;
  return synop.str();
}

void HqcDianaHelper::sendObservations()
{
#ifndef DO_NOT_SEND_ANY_DATA
  METLIBS_LOG_SCOPE();
  const timeutil::ptime& t = mDianaSensorTime.time;
  if (not mEnabled or not mDianaConnected or mSensors.empty() or t.is_not_a_date_time())
    return;

  HQC_LOG_ERROR("FIXME need to reimplement sendObservations");

  typedef std::map<int, Sensor> SensorByParam_t;
  typedef std::map<int, SensorByParam_t> SensorByStationAndParam_t;
  SensorByStationAndParam_t sendSensors;
  BOOST_FOREACH(const Sensor& s, mSensors)
      sendSensors[s.stationId].insert(std::make_pair(s.paramId, s)); // FIXME random results if more than one typeId/level/sensor

  std::ostringstream synopDescription;
  synopDescription << "id,St.type,auto,lon,lat";
  BOOST_FOREACH(const SendPars_t::value_type& p_sp, mSendPars) {
    const SendPar& sp = p_sp.second;
    synopDescription << ',' << sp.dianaName;
  }

  std::vector<std::string> synopData;
  BOOST_FOREACH(const SensorByStationAndParam_t::value_type& sps, sendSensors) {
    const int stationId = sps.first;
    const SensorByParam_t& sbp = sps.second;

    const std::string start = synopStart(stationId);
    if (start.empty())
      continue;

    std::ostringstream synopStation;
    synopStation << start;

    bool hasData = false;
    BOOST_FOREACH(const SendPars_t::value_type& p_sp, mSendPars) {
      const int paramId = p_sp.first;
      const SendPar& sp = p_sp.second;

      SensorByParam_t::const_iterator it = sbp.find(paramId);
      if (it != sbp.end()) {
        const Sensor& sensor = it->second;
        const SensorTime st(sensor, t);
        synopStation << synopValue(st, sp, hasData);
      } else {
        synopStation << ",-32767";
      }
    }

    if (hasData)
      synopData.push_back(synopStation.str());
  }

  if (not synopData.empty()) {
    miMessage pLetter;
    pLetter.command = mDianaNeedsHqcInit ? qmstrings::init_HQC_params : qmstrings::update_HQC_params;
    pLetter.commondesc = "time,plottype";
    pLetter.common = timeutil::to_iso_extended_string(mDianaSensorTime.time) + ",synop";

    pLetter.description = synopDescription.str();
    pLetter.data = synopData;
    sendMessage(pLetter);

    if (mDianaNeedsHqcInit)
      sendTimes();
    mDianaNeedsHqcInit = false;
  }
#endif // DO_NOT_SEND_ANY_DATA
}

void HqcDianaHelper::sendSelectedParam()
{
#ifndef DO_NOT_SEND_ANY_DATA
  METLIBS_LOG_SCOPE();
  if (not mEnabled or not mDianaConnected)
    return;

  const int paramId = mDianaSensorTime.sensor.paramId;
  SendPars_t::const_iterator it = mSendPars.find(paramId);
  if (it == mSendPars.end()) {
    HQC_LOG_WARN("No such diana parameter: " << paramId);
    return;
  }

  miMessage m;
  m.command = qmstrings::select_HQC_param;
  m.commondesc = "diParam";
  m.common     = it->second.dianaName;
  sendMessage(m);
#endif // DO_NOT_SEND_ANY_DATA
}

void HqcDianaHelper::sendMessage(miMessage& m)
{
  METLIBS_LOG_DEBUG(LOGVAL(m.content()));
  mClientButton->sendMessage(m);
}

// Help function to translate from kvalobs parameter value to diana parameter value
double HqcDianaHelper::dianaValue(int parNo, bool isModel, double qVal, double aa)
{
  double dVal;
  if ( parNo == 273 ) {
    if ( qVal <= 5000 )
      dVal = int(qVal)/100;
    else if ( qVal <= 30000 )
      dVal = int(qVal)/1000 + 50;
    else if ( qVal < 75000 )
      dVal = int(qVal)/5000 + 74;
    else
      dVal = 89;
    return dVal;
  }
  else if ( parNo == 55 ) {
    if ( qVal < 50 )
      dVal = 0;
    else if ( qVal < 100 )
      dVal = 1;
    else if ( qVal < 200 )
      dVal = 2;
    else if ( qVal < 300 )
      dVal = 3;
    else if ( qVal < 600 )
      dVal = 4;
    else if ( qVal < 1000 )
      dVal = 5;
    else if ( qVal < 1500 )
      dVal = 6;
    else if ( qVal < 2000 )
      dVal = 7;
    else if ( qVal < 2500 )
      dVal = 8;
    else
      dVal = 9;
    return dVal;
  }
  else if ( parNo == 177 ) {
    if ( aa >= 5.0 && !isModel ) {
      dVal = -qVal;
    }
    else {
      dVal = qVal;
    }
    return dVal;
  }
  //  else if ( parNo > 80 && parNo < 95 ) {
  //    dVal = 1.94384*qVal;
  //    return dVal;
  //  }
  else
    return qVal;
}

std::string HqcDianaHelper::hqcType(int typeId, int env)
{
  std::string hqct = "none";
  if ( env == 8 ) {
    if ( typeId == 3 || typeId == 330 || typeId == 342 )
      hqct = "AA";
    else if ( typeId == 1 || typeId == 6 || typeId == 312 || typeId == 412 )
      hqct = "VS";
    else if ( typeId == 306 )
      hqct = "VM";
  }
  else if ( env == 2 ) {
    if ( typeId == 3 )
      hqct = "AL";
  }
  else if ( env == 12 ) {
    if ( typeId == 3 )
      hqct = "AV";
  }
  else if ( env == 3 ) {
    if ( typeId == 3 || typeId == 410 )
      hqct = "AP";
    else if ( typeId == 412 )
      hqct = "VK";
  }
  else if ( env == 1 ) {
    if ( typeId == 310 || typeId == 311 || typeId == 410 || typeId == 1 )
      hqct = "AF";
    else if ( typeId == 2 )
      hqct = "FM";
    else if ( typeId == 1 || typeId == 6 || typeId == 312 || typeId == 412 )
      hqct = "VS";
  }
  else if ( env == 5 ) {
    if ( typeId == 6 || typeId == 430 )
      hqct = "AE";
    else if ( typeId == 11 )
      hqct = "MP";
  }
  else if ( env == 7 ) {
    if ( typeId == 11 )
      hqct = "MV";
  }
  else if ( env == 4 ) {
    if ( typeId == 11 )
      hqct = "MM";
  }
  else if ( env == 6 ) {
    if ( typeId == 11 )
      hqct = "MS";
  }
  else if ( env == 9 ) {
    if ( typeId == 302 || typeId == 303 )
      hqct = "NS";
    else if ( typeId == 402 )
      hqct = "ND";
    else if ( typeId == 4 || typeId == 405 )
      hqct = "P ";
  }
  else if ( env == 10 ) {
    if ( typeId == 402 )
      hqct = "NO";
  }
  else if ( env == 11 ) {
    if ( typeId == 309 )
      hqct = "VT";
  }
  return hqct;
}

void HqcDianaHelper::updateDianaParameters()
{
  METLIBS_LOG_SCOPE();
  mSendPars.clear();

  const char* item = "TTT";
  //if (mDianaConfigDialog->tameType->isChecked())
  //  mSendPars.insert(std::make_pair(212/*TAM*/, SendPar(item)));
  //else if (mDianaConfigDialog->tamoType->isChecked())
  //  mSendPars.insert(std::make_pair(211/*TA*/, SendPar(item, SendPar::MODEL)));
  //else if (mDianaConfigDialog->tadiType->isChecked())
  //  mSendPars.insert(std::make_pair(211/*TA*/, SendPar(item, SendPar::DIFF)));
  //else
    mSendPars.insert(std::make_pair(211, SendPar(item)));

  item = "TdTdTd";
  //if (mDianaConfigDialog->uuType->isChecked())
  //  mSendPars.insert(std::make_pair(262/*UU*/, SendPar(item)));
  //else if ( mDianaConfigDialog->uumoType->isChecked())
  //  mSendPars.insert(std::make_pair(262/*UU*/, SendPar(item, SendPar::MODEL)));
  //else if (mDianaConfigDialog->uumeType->isChecked())
  //  mSendPars.insert(std::make_pair(263/*UM*/, SendPar(item)));
  //else if (mDianaConfigDialog->uumiType->isChecked())
  //  mSendPars.insert(std::make_pair(264/*UN*/, SendPar(item)));
  //else if (mDianaConfigDialog->uumaType->isChecked())
  //  mSendPars.insert(std::make_pair(265/*UX*/, SendPar(item)));
  //else
    mSendPars.insert(std::make_pair(217/*TD*/, SendPar(item)));

  item = "PPPP";
  //if (mDianaConfigDialog->poType->isChecked())
  //  mSendPars.insert(std::make_pair(173/*PO*/, SendPar(item)));
  //else if (mDianaConfigDialog->prmoType->isChecked())
  //  mSendPars.insert(std::make_pair(173/*PO*/, SendPar(item, SendPar::MODEL)));
  //else if (mDianaConfigDialog->podiType->isChecked())
  //  mSendPars.insert(std::make_pair(173/*PO*/, SendPar(item, SendPar::DIFF)));
  //else if (mDianaConfigDialog->pomeType->isChecked())
  //  mSendPars.insert(std::make_pair(174/*POM*/, SendPar(item)));
  //else if (mDianaConfigDialog->pomiType->isChecked())
  //  mSendPars.insert(std::make_pair(175/*PON*/, SendPar(item)));
  //else if (mDianaConfigDialog->pomaType->isChecked())
  //  mSendPars.insert(std::make_pair(176/*POX*/, SendPar(item)));
  //else if (mDianaConfigDialog->phType->isChecked())
  //  mSendPars.insert(std::make_pair(172/*PH*/, SendPar(item)));
  //else
    mSendPars.insert(std::make_pair(178/*PR*/, SendPar(item)));

  item = "ppp";
  //if (mDianaConfigDialog->ppmoType->isChecked())
  //  mSendPars.insert(std::make_pair(177/*PP*/, SendPar(item, SendPar::MODEL)));
  //else
    mSendPars.insert(std::make_pair(177/*PP*/, SendPar(item)));

  item = "RRR";
  //if (mDianaConfigDialog->rrmoType->isChecked())
  //  mSendPars.insert(std::make_pair(109/*RR_12*/, SendPar(item, SendPar::MODEL)));
  //else if (mDianaConfigDialog->rr1Type->isChecked())
  //  mSendPars.insert(std::make_pair(106/*RR_1*/, SendPar(item)));
  //else if (mDianaConfigDialog->rr6Type->isChecked())
  //  mSendPars.insert(std::make_pair(108/*RR_6*/, SendPar(item)));
  //else if (mDianaConfigDialog->rr24Type->isChecked())
  //  mSendPars.insert(std::make_pair(110/*RR_24*/, SendPar(item)));
  //else if (mDianaConfigDialog->rr24moType->isChecked())
  //  mSendPars.insert(std::make_pair(110/*RR_24*/, SendPar(item, SendPar::MODEL)));
  //else if (mDianaConfigDialog->rrprType->isChecked())
  //  mSendPars.insert(std::make_pair(109/*RR_12*/, SendPar(item, SendPar::PROPORTION)));
  //else
    mSendPars.insert(std::make_pair(109/*RR_12*/, SendPar(item)));

  item = "TxTn";
  //if (mDianaConfigDialog->tx12Type->isChecked())
  //  mSendPars.insert(std::make_pair(216/*TAX_12*/, SendPar(item)));
  //else if (mDianaConfigDialog->tnType->isChecked())
  //  mSendPars.insert(std::make_pair(213/*TAN*/, SendPar(item)));
  //else if (mDianaConfigDialog->txType->isChecked())
  //  mSendPars.insert(std::make_pair(215/*TAX*/, SendPar(item)));
  //else
    mSendPars.insert(std::make_pair(214/*TAN_12*/, SendPar(item)));

  item = "dd";
  //if (mDianaConfigDialog->ddmoType->isChecked())
  //  mSendPars.insert(std::make_pair(61/*DD*/, SendPar(item, SendPar::MODEL)));
  //else
    mSendPars.insert(std::make_pair(61/*DD*/, SendPar(item)));

  item = "ff";
  //if (mDianaConfigDialog->ffmoType->isChecked())
  //  mSendPars.insert(std::make_pair(81/*FF*/, SendPar(item, SendPar::MODEL)));
  //else
    mSendPars.insert(std::make_pair(81/*FF*/, SendPar(item)));

  item = "fxfx";
  //if (mDianaConfigDialog->fx01Type->isChecked())
  //  mSendPars.insert(std::make_pair(87/*FX_1*/, SendPar(item)));
  //else
    mSendPars.insert(std::make_pair(86/*FX*/, SendPar(item)));

  item = "ff_911";
  //if (mDianaConfigDialog->fg01Type->isChecked())
  //  mSendPars.insert(std::make_pair(90/*FG_1*/, SendPar(item)));
  //else if (mDianaConfigDialog->fg10Type->isChecked())
  //  mSendPars.insert(std::make_pair(84/*FG_10*/, SendPar(item)));
  //else
    mSendPars.insert(std::make_pair(83/*FG*/, SendPar(item)));

  item = "sss";
  //if (mDianaConfigDialog->sdType->isChecked())
  //  mSendPars.insert(std::make_pair(18/*SD*/, SendPar(item)));
  //else if (mDianaConfigDialog->emType->isChecked())
  //  mSendPars.insert(std::make_pair(7/*EM*/, SendPar(item)));
  //else
    mSendPars.insert(std::make_pair(112/*SA*/, SendPar(item)));

  mSendPars.insert(std::make_pair( 55/*HL*/,     SendPar("h")));
  mSendPars.insert(std::make_pair( 15/*NN*/,     SendPar("N")));
  mSendPars.insert(std::make_pair(  1/*AA*/,     SendPar("a")));
  mSendPars.insert(std::make_pair( 41/*WW*/,     SendPar("ww")));
  mSendPars.insert(std::make_pair( 14/*NH*/,     SendPar("Nh")));
  mSendPars.insert(std::make_pair( 23/*CL*/,     SendPar("Cl")));
  mSendPars.insert(std::make_pair( 24/*CM*/,     SendPar("Cm")));
  mSendPars.insert(std::make_pair( 22/*CH*/,     SendPar("Ch")));
  mSendPars.insert(std::make_pair(242/*TW*/,     SendPar("TwTwTw")));
  mSendPars.insert(std::make_pair(273/*VV*/,     SendPar("VV")));
  mSendPars.insert(std::make_pair( 42/*W1*/,     SendPar("W1")));
  mSendPars.insert(std::make_pair( 43/*W2*/,     SendPar("W2")));
  mSendPars.insert(std::make_pair(134/*HWA*/,    SendPar("HwaHwa")));
  mSendPars.insert(std::make_pair(154/*PWA*/,    SendPar("PwaPwa")));
  mSendPars.insert(std::make_pair( 65/*DW1*/,    SendPar("dw1dw1")));
  mSendPars.insert(std::make_pair(152/*PW1*/,    SendPar("Pw1Pw1")));
  mSendPars.insert(std::make_pair(132/*HW1*/,    SendPar("Hw1Hw1")));
  mSendPars.insert(std::make_pair( 19/*SG*/,     SendPar("s")));
  mSendPars.insert(std::make_pair(403/*MDIR*/,   SendPar("ds")));
  mSendPars.insert(std::make_pair(404/*MSPEED*/, SendPar("vs")));
}
