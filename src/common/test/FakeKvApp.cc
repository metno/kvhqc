/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "FakeKvApp.hh"
#include "FakeReinserter.hh"

//#include "KvalobsAccess.hh"
//#include "KvHelpers.hh"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#define MILOGGER_CATEGORY "kvhqc.test.FakeKvApp"
#include "util/HqcLogging.hh"

static float toFloat(const std::string& txt)
{
  if (txt == "null")
    return -99999;
  else
    return boost::lexical_cast<float>(txt);
}

FakeKvApp::FakeKvApp(bool useThread)
{
  mObsAccess = std::make_shared<SqliteAccess>(useThread);

  mObsAccess->insertParam(kvalobs::kvParam(18, "SD", "Snødekke", "nasjonal kode ett siffer", 0, "Verdien -1 angir at snødekke ikke er meldt"));
  mObsAccess->insertParam(kvalobs::kvParam(34, "V4", "Været siden forrige hovedobservasjon, første tegn", "nasjonal kode to siffer", 0, "None"));
  mObsAccess->insertParam(kvalobs::kvParam(35, "V4S", "Værkodens styrke. Tilhører V4", "nasjonal kode ett siffer", 0, "None"));
  mObsAccess->insertParam(kvalobs::kvParam(36, "V5", "Været siden forrige hovedobservasjon, andre tegn", "nasjonal kode to siffer", 0, "None"));
  mObsAccess->insertParam(kvalobs::kvParam(37, "V5S", "Værkodens styrke. Tilhører V5", "nasjonal kode ett siffer", 0, "None"));
  mObsAccess->insertParam(kvalobs::kvParam(38, "V6", "Været siden forrige hovedobservasjon, tredje tegn", "nasjonal kode to siffer", 0, "None"));
  mObsAccess->insertParam(kvalobs::kvParam(39, "V6S", "Værkodens styrke. Tilhører V6", "nasjonal kode ett siffer", 0, "None"));
  mObsAccess->insertParam(kvalobs::kvParam(105, "RR_01", "Nedbør, tilvekst siste minutt", "mm", 0, "1 min akkumulert"));
  mObsAccess->insertParam(kvalobs::kvParam(106, "RR_1", "Nedbør, tilvekst siste time", "mm", 0, "Verdien -1 angir at nedbørmengde ikke er meldt"));
  mObsAccess->insertParam(kvalobs::kvParam(107, "RR_3", "Nedbør, tilvekst siste 3 timer", "mm", 0, "Verdien -1 angir at nedbørmengde ikke er meldt"));
  mObsAccess->insertParam(kvalobs::kvParam(108, "RR_6", "Nedbør, tilvekst siste 6 timer", "mm", 0, "Verdien -1 angir at nedbørmengde ikke er meldt"));
  mObsAccess->insertParam(kvalobs::kvParam(109, "RR_12", "Nedbør, tilvekst siste 12 timer", "mm", 0, "Verdien -1 angir at nedbørmengde ikke er meldt"));
  mObsAccess->insertParam(kvalobs::kvParam(110, "RR_24", "Nedbør, tilvekst siste 24 timer", "mm", 0, "Verdien -1 angir at nedbørmengde ikke er meldt"));
  mObsAccess->insertParam(kvalobs::kvParam(173, "PO", "Trykk QFE", "hPa", 0, "Trykket i stasjonsnivå, ett minutt nåverdi"));
  mObsAccess->insertParam(kvalobs::kvParam(178, "PR", "Trykk QFF", "hPa", 0, "Trykket redusert til havets nivå m/lufttemperatur nå, nåverdi. Inngår i SYNOP"));
  mObsAccess->insertParam(kvalobs::kvParam(211, "TA", "Temperatur", "°C", 0, "Nåverdi"));
  mObsAccess->insertParam(kvalobs::kvParam(212, "TAM", "Temperatur timemiddel", "°C", 0, "None"));
  mObsAccess->insertParam(kvalobs::kvParam(213, "TAN", "Temperatur minimum i timen", "°C", 0, "minimum minuttverdi i timen"));
  mObsAccess->insertParam(kvalobs::kvParam(214, "TAN_12", "Temperatur minimum siste 12 timer", "°C", 0, "minimum minuttverdi siste 12 timer"));
  mObsAccess->insertParam(kvalobs::kvParam(215, "TAX", "Temperatur maksimum i timen", "°C", 0, "maksimum minuttverdi i timen"));
  mObsAccess->insertParam(kvalobs::kvParam(216, "TAX_12", "Temperatur maksimum siste 12 timer", "°C", 0, "maksimum minuttverdi siste 12 timer"));
  mObsAccess->insertParam(kvalobs::kvParam(262, "UU", "Relativ luftfuktighet", "%", 0, "Nåverdi"));
}

FakeKvApp::~FakeKvApp()
{
}

void FakeKvApp::insertData(int stationId, int paramId, int typeId, const std::string& obstime, float orig, float corr,
    const std::string& controlinfo, const std::string& cfailed)
{
  const timeutil::ptime tbtime = boost::posix_time::microsec_clock::universal_time();
  obsAccess()->insertData(kvalobs::kvData(stationId, timeutil::to_miTime(timeutil::from_iso_extended_string(obstime)), orig,
          paramId, timeutil::to_miTime(tbtime), typeId, 0, 0, corr,
          controlinfo, kvalobs::kvUseInfo(), cfailed));
}

void FakeKvApp::insertDataFromFile(const std::string& filename)
{
  obsAccess()->insertDataFromFile(filename);
}

void FakeKvApp::insertModel(int stationId, int paramId, const std::string& obstime, float value)
{
  const kvalobs::kvModelData kvm(stationId, timeutil::to_miTime(timeutil::from_iso_extended_string(obstime)), paramId, 0, 0, value);
  obsAccess()->insertModel(kvm);
}

void FakeKvApp::insertModelFromFile(const std::string& filename)
{
  obsAccess()->insertModelFromFile(filename);
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
    const float lon   = toFloat(columns[c++]);
    const float lat   = toFloat(columns[c++]);
    const int height  = toFloat(columns[c++]);
    const std::string name = columns[c++];
    const int env = boost::lexical_cast<int>(columns[c++]);
    const timeutil::ptime from = timeutil::from_iso_extended_string(columns[c++]);
        
    obsAccess()->insertStation(kvalobs::kvStation(station, lat, lon, height, 0.0f, name, 0, 0, "?", "?", "?", env, true, from));
  } catch (std::exception& e) {
    HQC_LOG_WARN("error parsing station line '" << line << "': " << e.what());
  }
}

void FakeKvApp::addObsPgm(const std::string& line)
{
  if (line.empty() or line.at(0) == '#' or line.at(0) == ' ')
    return;
    
  std::vector<std::string> columns;
  boost::split(columns, line, boost::is_any_of("\t;"));
  int ipm; // offset due to priority_message
  if (columns.size() == 39) {
    ipm = 0;
  } else if (columns.size() == 40) {
    ipm = 1;
  } else {
    HQC_LOG_WARN("bad line '" << line << "' cols=" << columns.size());
    return;
  }
  int numbers[38];
  for (int c = 0; c < 37 + ipm; c++)
    numbers[c] = boost::lexical_cast<int>(columns[c]);
  timeutil::ptime from, to;
  if (columns[37 + ipm].size() == 19)
    from = timeutil::from_iso_extended_string(columns[37 + ipm]);
  if (columns[38 + ipm].size() == 19)
    from = timeutil::from_iso_extended_string(columns[38 + ipm]);

  const bool priority_message = (ipm == 1) ? (numbers[5] != 0) : true;

  const hqc::hqcObsPgm op(numbers[0], numbers[1], numbers[2], numbers[3], numbers[4], priority_message, numbers[5 + ipm], numbers[6 + ipm], numbers[7 + ipm],
                          numbers[8 + ipm], numbers[9 + ipm], numbers[10 + ipm], numbers[11], numbers[12 + ipm], numbers[13 + ipm], numbers[14 + ipm],
                          numbers[15 + ipm], numbers[16 + ipm], numbers[17], numbers[18 + ipm], numbers[19 + ipm], numbers[20 + ipm], numbers[21 + ipm],
                          numbers[22 + ipm], numbers[23], numbers[24 + ipm], numbers[25 + ipm], numbers[26 + ipm], numbers[27 + ipm], numbers[28 + ipm],
                          numbers[29], numbers[30 + ipm], numbers[31 + ipm], numbers[32 + ipm], numbers[33 + ipm], numbers[34 + ipm], numbers[35],
                          numbers[36 + ipm], from, to);
  obsAccess()->insertObsPgm(op);
}

bool FakeKvApp::getKvData(kvservice::KvGetDataReceiver&, const kvservice::WhichDataHelper&)
{
  return false;
}

bool FakeKvApp::getKvRejectDecode(const CKvalObs::CService::RejectDecodeInfo&, kvservice::RejectDecodeIterator&)
{
  return false;
}

bool FakeKvApp::getKvParams(std::list<kvalobs::kvParam> &paramList)
{
  paramList.clear();
  return false;
}

bool FakeKvApp::getKvStations(std::list<kvalobs::kvStation> &stationList)
{
  stationList.clear();
  return false;
}

bool FakeKvApp::getKvModelData(std::list<kvalobs::kvModelData> &dataList, const kvservice::WhichDataHelper&)
{
  dataList.clear();
  return false;
}

bool FakeKvApp::getKvReferenceStations(int, int, std::list<kvalobs::kvReferenceStation> &refList)
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

bool FakeKvApp::getKvStationParam(std::list<kvalobs::kvStationParam> &stParam, int, int, int)
{
  stParam.clear();
  return false;
}

bool FakeKvApp::getKvStationMetaData(std::list<kvalobs::kvStationMetadata> &stMeta,
    int, const kvtime_t&, const std::string&)
{
  stMeta.clear();
  return false;
}

bool FakeKvApp::getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm, const std::list<long>&, bool)
{
  obsPgm.clear();
  return false;
}

bool FakeKvApp::getKvData(kvservice::KvObsDataList &dataList, const kvservice::WhichDataHelper&)
{
  dataList.clear();
  return false;
}

bool FakeKvApp::getKvWorkstatistik(CKvalObs::CService::WorkstatistikTimeType,
    const kvtime_t&, const kvtime_t&, kvservice::WorkstatistikIterator&)
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
    
const CKvalObs::CDataSource::Result_var FakeKvApp::sendDataToKv(const char*, const char*)
{
  return makeResult(CKvalObs::CDataSource::ERROR);
}
    
kvservice::KvApp::SubscriberID FakeKvApp::subscribeDataNotify(const kvservice::KvDataSubscribeInfoHelper&, dnmi::thread::CommandQue&)
{
  return "heiho-data-notify";
}

kvservice::KvApp::SubscriberID FakeKvApp::subscribeData(const kvservice::KvDataSubscribeInfoHelper&, dnmi::thread::CommandQue&)
{
  return "heiho-data";
}

kvservice::KvApp::SubscriberID FakeKvApp::subscribeKvHint(dnmi::thread::CommandQue&)
{
  return "heiho-hint";
}

void FakeKvApp::unsubscribe(const kvservice::KvApp::SubscriberID&)
{
}

void FakeKvApp::unsubscribeAll()
{
}

FakeKvApp* FakeKvApp::app()
{
  return 0;
}
