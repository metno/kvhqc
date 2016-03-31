
#include "FakeKvApp.hh"
#include "FakeReinserter.hh"

#include "KvalobsAccess.hh"
#include "KvHelpers.hh"

#include <boost/foreach.hpp>
#include <memory>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>

#define MILOGGER_CATEGORY "kvhqc.test.FakeKvApp"
#include "util/HqcLogging.hh"

FakeKvApp::FakeKvApp()
{
  kvservice::KvApp::kvApp = this;
  kda = std::make_shared<KvalobsAccess>();
  kda->setReinserter(new FakeReinserter);

  mKvParams.push_back(kvalobs::kvParam(18, "SD", "Snødekke", "nasjonal kode ett siffer", 0, "Verdien -1 angir at snødekke ikke er meldt"));
  mKvParams.push_back(kvalobs::kvParam(34, "V4", "Været siden forrige hovedobservasjon, første tegn", "nasjonal kode to siffer", 0, "None"));
  mKvParams.push_back(kvalobs::kvParam(35, "V4S", "Værkodens styrke. Tilhører V4", "nasjonal kode ett siffer", 0, "None"));
  mKvParams.push_back(kvalobs::kvParam(36, "V5", "Været siden forrige hovedobservasjon, andre tegn", "nasjonal kode to siffer", 0, "None"));
  mKvParams.push_back(kvalobs::kvParam(37, "V5S", "Værkodens styrke. Tilhører V5", "nasjonal kode ett siffer", 0, "None"));
  mKvParams.push_back(kvalobs::kvParam(38, "V6", "Været siden forrige hovedobservasjon, tredje tegn", "nasjonal kode to siffer", 0, "None"));
  mKvParams.push_back(kvalobs::kvParam(39, "V6S", "Værkodens styrke. Tilhører V6", "nasjonal kode ett siffer", 0, "None"));
  mKvParams.push_back(kvalobs::kvParam(105, "RR_01", "Nedbør, tilvekst siste minutt", "mm", 0, "1 min akkumulert"));
  mKvParams.push_back(kvalobs::kvParam(106, "RR_1", "Nedbør, tilvekst siste time", "mm", 0, "Verdien -1 angir at nedbørmengde ikke er meldt"));
  mKvParams.push_back(kvalobs::kvParam(107, "RR_3", "Nedbør, tilvekst siste 3 timer", "mm", 0, "Verdien -1 angir at nedbørmengde ikke er meldt"));
  mKvParams.push_back(kvalobs::kvParam(108, "RR_6", "Nedbør, tilvekst siste 6 timer", "mm", 0, "Verdien -1 angir at nedbørmengde ikke er meldt"));
  mKvParams.push_back(kvalobs::kvParam(109, "RR_12", "Nedbør, tilvekst siste 12 timer", "mm", 0, "Verdien -1 angir at nedbørmengde ikke er meldt"));
  mKvParams.push_back(kvalobs::kvParam(110, "RR_24", "Nedbør, tilvekst siste 24 timer", "mm", 0, "Verdien -1 angir at nedbørmengde ikke er meldt"));
  mKvParams.push_back(kvalobs::kvParam(173, "PO", "Trykk QFE", "hPa", 0, "Trykket i stasjonsnivå, ett minutt nåverdi"));
  mKvParams.push_back(kvalobs::kvParam(178, "PR", "Trykk QFF", "hPa", 0, "Trykket redusert til havets nivå m/lufttemperatur nå, nåverdi. Inngår i SYNOP"));
  mKvParams.push_back(kvalobs::kvParam(211, "TA", "Temperatur", "°C", 0, "Nåverdi"));
  mKvParams.push_back(kvalobs::kvParam(212, "TAM", "Temperatur timemiddel", "°C", 0, "None"));
  mKvParams.push_back(kvalobs::kvParam(213, "TAN", "Temperatur minimum i timen", "°C", 0, "minimum minuttverdi i timen"));
  mKvParams.push_back(kvalobs::kvParam(214, "TAN_12", "Temperatur minimum siste 12 timer", "°C", 0, "minimum minuttverdi siste 12 timer"));
  mKvParams.push_back(kvalobs::kvParam(215, "TAX", "Temperatur maksimum i timen", "°C", 0, "maksimum minuttverdi i timen"));
  mKvParams.push_back(kvalobs::kvParam(216, "TAX_12", "Temperatur maksimum siste 12 timer", "°C", 0, "maksimum minuttverdi siste 12 timer"));
  mKvParams.push_back(kvalobs::kvParam(262, "UU", "Relativ luftfuktighet", "%", 0, "Nåverdi"));
}

FakeKvApp::~FakeKvApp()
{
  kvservice::KvApp::kvApp = 0;
}

void FakeKvApp::insertData(int stationId, int paramId, int typeId, const std::string& obstime, float orig, float corr,
    const std::string& controlinfo, const std::string& cfailed)
{
  const timeutil::ptime tbtime = boost::posix_time::microsec_clock::universal_time();
  const kvalobs::kvData data(stationId, timeutil::to_miTime(timeutil::from_iso_extended_string(obstime)), orig,
      paramId, timeutil::to_miTime(tbtime), typeId, 0, 0, corr,
      controlinfo, kvalobs::kvUseInfo(), cfailed);
  const SensorTime st(Helpers::sensorFromKvData(data), timeutil::from_iso_extended_string(obstime));
  const Data_t::iterator it = mData.find(st);
  if (it != mData.end())
    mData.erase(it);
  mData.insert(Data_t::value_type(st, data));
}

void FakeKvApp::insertDataFromFile(const std::string& filename)
{
  METLIBS_LOG_INFO("loading data from file '" << filename << "'");
  std::ifstream f(filename.c_str());
  std::string line;

  while (std::getline(f, line)) {
    if (line.empty() or line.at(0) == '#' or line.at(0) == ' ')
      continue;

    std::vector<std::string> columns;
    boost::split(columns, line, boost::is_any_of("\t"));
    if (columns.size() != 7 and columns.size() != 8) {
      HQC_LOG_WARN("bad line '" << line << "' cols=" << columns.size());
      continue;
    }

    unsigned int c = 0;
    const int stationId = boost::lexical_cast<int>(columns[c++]);
    const int paramId   = boost::lexical_cast<int>(columns[c++]);
    const int typeId    = boost::lexical_cast<int>(columns[c++]);
    const std::string obstime = columns[c++];
    const float original  = boost::lexical_cast<float>(columns[c++]);
    const float corrected = boost::lexical_cast<float>(columns[c++]);
    const std::string controlinfo = columns[c++];
    std::string cfailed;
    if (c<columns.size())
      cfailed = columns[c++];

    insertData(stationId, paramId, typeId, obstime, original, corrected, controlinfo, cfailed);
  }
}

bool FakeKvApp::eraseData(const SensorTime& st)
{
  Data_t::iterator it = mData.find(st);
  if (it != mData.end()) {
    mData.erase(it);
    return true;
  } else {
    return false;
  }
}

void FakeKvApp::insertModel(int stationId, int paramId, const std::string& obstime, float value)
{
  const kvalobs::kvModelData model(stationId, timeutil::to_miTime(timeutil::from_iso_extended_string(obstime)), paramId, 0, 0, value);
  const SensorTime st(Sensor(stationId, paramId, 0, 0, 0), timeutil::from_iso_extended_string(obstime));
  const ModelData_t::iterator it = mModelData.find(st);
  if (it != mModelData.end())
    mModelData.erase(it);
  mModelData.insert(ModelData_t::value_type(st, model));
}

void FakeKvApp::insertModelFromFile(const std::string& filename)
{
  METLIBS_LOG_INFO("loading model data from file '" << filename << "'");

  std::ifstream f(filename.c_str());
  std::string line;

  while (std::getline(f, line)) {
    if (line.empty() or line.at(0) == '#' or line.at(0) == ' ')
      continue;

    try {
      std::vector<std::string> columns;
      boost::split(columns, line, boost::is_any_of("\t"));
      if (columns.size() != 4) {
        HQC_LOG_WARN("bad model line '" << line << "' cols=" << columns.size());
        continue;
      }

      unsigned int c = 0;
      const int stationId = boost::lexical_cast<int>(columns[c++]);
      const int paramId   = boost::lexical_cast<int>(columns[c++]);
      const std::string obstime = columns[c++];
      const float value  = boost::lexical_cast<float>(columns[c++]);
            
      insertModel(stationId, paramId, obstime, value);
    } catch (std::exception& e) {
      HQC_LOG_WARN("error parsing model line '" << line << "' in file '" + filename + "'");
    }
  }
}

void FakeKvApp::addStation(const std::string& line)
{
  if (line.empty() or line.at(0) == '#' or line.at(0) == ' ')
    return;
    
  try {
    std::vector<std::string> columns;
    boost::split(columns, line, boost::is_any_of("\t;"));
    if (columns.size() != 7) {
      HQC_LOG_WARN("bad line '" << line << "' cols=" << columns.size());
      return;
    }
        
    unsigned int c = 0;
    const int station = boost::lexical_cast<int>(columns[c++]);
    const float lon   = boost::lexical_cast<float>(columns[c++]);
    const float lat   = boost::lexical_cast<float>(columns[c++]);
    const int height  = boost::lexical_cast<float>(columns[c++]);
    const std::string name = columns[c++];
    const int env = boost::lexical_cast<int>(columns[c++]);
    const timeutil::ptime from = timeutil::from_iso_extended_string(columns[c++]);
        
    // TODO check if station already exists
    if (std::find_if(mKvStations.begin(), mKvStations.end(), Helpers::stations_by_id(station)) == mKvStations.end())
      mKvStations.push_back(kvalobs::kvStation(station, lat, lon, height, 0.0f, name, 0, 0, "?", "?", "?", env, true, from));
  } catch (std::exception& e) {
    HQC_LOG_WARN("error parsing station line '" << line << "'");
  }
}

void FakeKvApp::addObsPgm(const std::string& line)
{
  if (line.empty() or line.at(0) == '#' or line.at(0) == ' ')
    return;
    
  std::vector<std::string> columns;
  boost::split(columns, line, boost::is_any_of("\t;"));
  if (columns.size() != 39) {
    HQC_LOG_WARN("bad line '" << line << "' cols=" << columns.size());
    return;
  }

  int numbers[37];
  for (int c=0; c<37; c++)
    numbers[c] = boost::lexical_cast<int>(columns[c]);
  timeutil::ptime from, to;
  if (columns[37].size() == 19)
    from = timeutil::from_iso_extended_string(columns[37]);
  if (columns[38].size() == 19)
    from = timeutil::from_iso_extended_string(columns[38]);

  mObsPgm.push_back(kvalobs::kvObsPgm(numbers[ 0], numbers[ 1], numbers[ 2], numbers[ 3], numbers[ 4], numbers[ 5],
          numbers[ 6], numbers[ 7], numbers[ 8], numbers[ 9], numbers[10], numbers[11],
          numbers[12], numbers[13], numbers[14], numbers[15], numbers[16], numbers[17],
          numbers[18], numbers[19], numbers[20], numbers[21], numbers[22], numbers[23],
          numbers[24], numbers[25], numbers[26], numbers[27], numbers[28], numbers[29],
          numbers[30], numbers[31], numbers[32], numbers[33], numbers[34], numbers[35], numbers[36],
          from, to));
}

bool FakeKvApp::eraseModel(const SensorTime& st)
{
  ModelData_t::iterator it = mModelData.find(st);
  if (it != mModelData.end()) {
    mModelData.erase(it);
    return true;
  } else {
    return false;
  }
}

bool FakeKvApp::getKvData(kvservice::KvGetDataReceiver &dataReceiver, const kvservice::WhichDataHelper &wd)
{
  kvservice::KvObsDataList dataList;
  if (getKvData(dataList, wd)) {
    dataReceiver.next(dataList);
    return true;
  } else {
    return false;
  }
}

bool FakeKvApp::getKvRejectDecode(const CKvalObs::CService::RejectDecodeInfo &decodeInfo, kvservice::RejectDecodeIterator &it)
{
  return false;
}

bool FakeKvApp::getKvParams(std::list<kvalobs::kvParam> &paramList)
{
  paramList = mKvParams;
  return true;
}

bool FakeKvApp::getKvStations(std::list<kvalobs::kvStation> &stationList)
{
  stationList = mKvStations;
  return true;
}

bool FakeKvApp::getKvModelData(std::list<kvalobs::kvModelData> &dataList, const kvservice::WhichDataHelper &wd )
{
  METLIBS_LOG_SCOPE();
  dataList.clear();

  const int BIG = 999999;
  const CKvalObs::CService::WhichDataList& whichData = *wd.whichData();
  for (long wi = 0; wi < whichData.length(); ++wi) {
    const ModelData_t::iterator low  = mModelData.lower_bound(SensorTime(Sensor(whichData[wi].stationid,   0,   0,   0, -BIG),
            timeutil::from_iso_extended_string("1500-01-01 00:00:00")));
    const ModelData_t::iterator high = mModelData.upper_bound(SensorTime(Sensor(whichData[wi].stationid, BIG, BIG, BIG,  BIG),
            timeutil::from_iso_extended_string("2500-01-01 00:00:00")));
    const timeutil::ptime fromTime = timeutil::from_iso_extended_string(std::string(whichData[wi].fromObsTime));
    const timeutil::ptime toTime   = timeutil::from_iso_extended_string(std::string(whichData[wi].toObsTime));
    const bool hasToTime = (not toTime.is_not_a_date_time());

    for (ModelData_t::iterator it = low; it != high; ++it) {
      const kvalobs::kvModelData& data = it->second;
      const timeutil::ptime ot = timeutil::from_miTime(data.obstime());
      if (fromTime <= ot and ((not hasToTime) or (ot <= toTime))) {
        METLIBS_LOG_DEBUG(LOGVAL(data));
        dataList.push_back(data);
      }
    }
  }
  METLIBS_LOG_DEBUG(LOGVAL(dataList.size()));
  return true;
}

bool FakeKvApp::getKvReferenceStations(int stationid, int paramid, std::list<kvalobs::kvReferenceStation> &refList)
{
  refList.clear();
  return false;
}

bool FakeKvApp::getKvTypes(std::list<kvalobs::kvTypes> &typeList)
{
  typeList.clear();
  return false;
}

bool FakeKvApp::getKvOperator(std::list<kvalobs::kvOperator> &operatorList)
{
  operatorList.clear();
  return false;
}

bool FakeKvApp::getKvStationParam(std::list<kvalobs::kvStationParam> &stParam, int stationid, int paramid, int day)
{
  stParam.clear();
  return false;
}

bool FakeKvApp::getKvStationMetaData(std::list<kvalobs::kvStationMetadata> &stMeta,
    int stationid, const kvtime_t &obstime,
    const std::string & metadataName)
{
  stMeta.clear();
  return false;
}

bool FakeKvApp::getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm, const std::list<long> &stationList, bool aUnion)
{
  if (aUnion)
    return false;

  obsPgm.clear();
  std::set<long int> stations(stationList.begin(), stationList.end());
  BOOST_FOREACH(const kvalobs::kvObsPgm& op, mObsPgm) {
    if (stations.find(op.stationID()) != stations.end())
      obsPgm.push_back(op);
  }
  return true;
}

bool FakeKvApp::getKvData(kvservice::KvObsDataList &dataList, const kvservice::WhichDataHelper &wd)
{
  METLIBS_LOG_SCOPE();
  dataList.clear();

  const CKvalObs::CService::WhichDataList& whichData = *wd.whichData();
  for (long wi = 0; wi < whichData.length(); ++wi) {
    const long sid = whichData[wi].stationid;
    const timeutil::ptime fromTime = timeutil::from_iso_extended_string(std::string(whichData[wi].fromObsTime));
    const timeutil::ptime toTime   = timeutil::from_iso_extended_string(std::string(whichData[wi].toObsTime));
    const bool hasToTime = (not toTime.is_not_a_date_time());
    METLIBS_LOG_DEBUG(LOGVAL(sid) << LOGVAL(fromTime) << LOGVAL(toTime) << LOGVAL(hasToTime));

    kvservice::KvObsData od;
    BOOST_FOREACH(const Data_t::value_type& v, mData) {
      const kvalobs::kvData& data = v.second;
      if (data.stationID() != sid)
        continue;
      const timeutil::ptime ot = timeutil::from_miTime(data.obstime());
      if (fromTime > ot)
        continue;
      if (hasToTime and ot > toTime)
        continue;
      od.dataList().push_back(data);
      METLIBS_LOG_DEBUG(LOGVAL(data));
    }
    dataList.push_back(od);
  }
  return true;
}

bool FakeKvApp::getKvWorkstatistik(CKvalObs::CService::WorkstatistikTimeType timeType,
    const kvtime_t &from, const kvtime_t &to,
    kvservice::WorkstatistikIterator &it)
{
  return false;
}

static const CKvalObs::CDataSource::Result_var makeResult(CKvalObs::CDataSource::EResult what)
{
  // FIXME same as in FakeReinserter
  CKvalObs::CDataSource::Result_var ret(new CKvalObs::CDataSource::Result);
  ret->res = what;
  ret->message = "FakeKvApp response";
  return ret;
}
    
const CKvalObs::CDataSource::Result_var FakeKvApp::sendDataToKv(const char *data, const char *obsType)
{
  return makeResult(CKvalObs::CDataSource::ERROR);
}
    
kvservice::KvApp::SubscriberID FakeKvApp::subscribeDataNotify(const kvservice::KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que)
{
  return "heiho-data-notify";
}

kvservice::KvApp::SubscriberID FakeKvApp::subscribeData(const kvservice::KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que)
{
  return "heiho-data";
}

kvservice::KvApp::SubscriberID FakeKvApp::subscribeKvHint(dnmi::thread::CommandQue &que)
{
  return "heiho-hint";
}

void FakeKvApp::unsubscribe(const kvservice::KvApp::SubscriberID &subscriberid)
{
}

void FakeKvApp::unsubscribeAll()
{
}
