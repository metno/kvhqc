
#include "KvServiceHelper.hh"

#include "TimeRange.hh"

#include <kvalobs/kvStationParam.h>
#include <kvcpp/KvApp.h>

#include <puTools/miString.h>

#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtGui/QIcon>
#include <QtGui/QMessageBox>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.KvServiceHelper"
#include "util/HqcLogging.hh"

KvServiceHelper* KvServiceHelper::sInstance = 0;

KvServiceHelper::KvServiceHelper()
    : mKvalobsAvailable(true)
{
  sInstance = this;
}

KvServiceHelper::~KvServiceHelper()
{
  sInstance = 0;
}

bool KvServiceHelper::checkKvalobsAvailability()
{
  std::list<kvalobs::kvStationParam> stParam;
  return updateKvalobsAvailability(kvservice::KvApp::kvApp->getKvStationParam(stParam, 345345, 345345, 0));
}

bool KvServiceHelper::updateKvalobsAvailability(bool available)
{
  if (available != mKvalobsAvailable) {
    mKvalobsAvailable = available;
    Q_EMIT kvalobsAvailable(mKvalobsAvailable);
  }
  return available;
}

namespace /* anonymous */ {

std::string whichDataString(const kvservice::WhichDataHelper& wd)
{
  std::ostringstream msg;
  const CKvalObs::CService::WhichDataList& wdl = *wd.whichData();
  for(unsigned long i = 0; i<wdl.length(); ++i) {
    const CKvalObs::CService::WhichData& wdi = wdl[i];
    msg << '[' << wdi.stationid << ':' << wdi.fromObsTime << '-' << wdi.toObsTime << ']';
  }
  return msg.str();
}

std::string stationListString(const std::list<long>& stationids)
{
  std::ostringstream msg;
  std::copy(stationids.begin(), stationids.end(), std::ostream_iterator<long>(msg, ", "));
  return msg.str();
}

} // namespace anonymous

bool KvServiceHelper::getKvData(kvservice::KvGetDataReceiver& dataReceiver, const kvservice::WhichDataHelper& wd)
{
  METLIBS_LOG_SCOPE();
  try {
    return updateKvalobsAvailability(kvservice::KvApp::kvApp->getKvData(dataReceiver, wd));
  } catch (std::exception& e) {
    HQC_LOG_ERROR("kvalobs exception in getKvData(..," << whichDataString(wd) << "); message is: " << e.what());
    updateKvalobsAvailability(false);
    throw e;
  } catch (...) {
    HQC_LOG_ERROR("kvalobs exception in getKvData(..," << whichDataString(wd) << ')');
    updateKvalobsAvailability(false);
    throw;
  }
}

bool KvServiceHelper::getKvModelData(std::list<kvalobs::kvModelData> &dataList, const kvservice::WhichDataHelper& wd)
{
  METLIBS_LOG_SCOPE();
  try {
    return updateKvalobsAvailability(kvservice::KvApp::kvApp->getKvModelData(dataList, wd));
  } catch (std::exception& e) {
    HQC_LOG_ERROR("kvalobs exception in getKvModelData(..," << whichDataString(wd) << "); message is: " << e.what());
    updateKvalobsAvailability(false);
    throw e;
  } catch (...) {
    HQC_LOG_ERROR("kvalobs exception in getKvModelData(..," << whichDataString(wd) << ')');
    updateKvalobsAvailability(false);
    throw;
  }
}

bool KvServiceHelper::getKvRejectDecode(std::list<kvalobs::kvRejectdecode>& rejectList, const TimeRange& timeLimits)
{
  METLIBS_LOG_SCOPE();
  rejectList.clear();

  CKvalObs::CService::RejectDecodeInfo rdInfo;
  rdInfo.fromTime = timeutil::to_iso_extended_string(timeLimits.t0()).c_str();
  rdInfo.toTime   = timeutil::to_iso_extended_string(timeLimits.t1()).c_str();

  try {
    kvservice::RejectDecodeIterator rdIt;
    const bool ok = kvservice::KvApp::kvApp->getKvRejectDecode(rdInfo, rdIt);
    updateKvalobsAvailability(ok);
    if (ok) {
      kvalobs::kvRejectdecode reject;
      while (rdIt.next(reject))
        rejectList.push_back(reject);
    }
    return ok;
  } catch (std::exception& e) {
    HQC_LOG_ERROR("kvalobs exception in 'getKvRejectDecode' for time " << timeLimits << ": " << e.what());
    updateKvalobsAvailability(false);
    throw e;
  } catch (...) {
    HQC_LOG_ERROR("kvalobs exception in 'getKvRejectDecode' for time " << timeLimits);
    updateKvalobsAvailability(false);
    throw;
  }
}

bool KvServiceHelper::getKvObsPgm(std::list<kvalobs::kvObsPgm>& obsPgm, const std::list<long>& stationList)
{
  METLIBS_LOG_SCOPE();
  try {
    return updateKvalobsAvailability(kvservice::KvApp::kvApp->getKvObsPgm(obsPgm, stationList, false));
  } catch (std::exception& e) {

    HQC_LOG_ERROR("kvalobs exception in getKvObsPgm(..," << stationListString(stationList) << ", false); message is: " << e.what());
    updateKvalobsAvailability(false);
    throw e;
  } catch (...) {
    HQC_LOG_ERROR("kvalobs exception in getKvObsPgm(..," << stationListString(stationList) << ", false)");
    updateKvalobsAvailability(false);
    throw;
  }
}

#define TRY_KVALOBS(func, args)                                         \
  METLIBS_LOG_SCOPE();                                                  \
  try {                                                                 \
    return updateKvalobsAvailability(kvservice::KvApp::kvApp->func args); \
  } catch (std::exception& e) {                                         \
    HQC_LOG_ERROR("kvalobs exception in '" #func "': " << e.what()); \
    updateKvalobsAvailability(false);                                   \
    throw e;                                                            \
  } catch (...) {                                                       \
    HQC_LOG_ERROR("kvalobs exception in '" #func "'");              \
    updateKvalobsAvailability(false);                                   \
    throw;                                                              \
  }                                                                     \

bool KvServiceHelper::getKvParams(std::list<kvalobs::kvParam>& paramList)
{
  TRY_KVALOBS(getKvParams, (paramList));
}

bool KvServiceHelper::getKvStations( std::list<kvalobs::kvStation>& stationList)
{
  TRY_KVALOBS(getKvStations, (stationList));
}

bool KvServiceHelper::getKvTypes(std::list<kvalobs::kvTypes>& typeList)
{
  TRY_KVALOBS(getKvTypes, (typeList));
}

bool KvServiceHelper::getKvOperator(std::list<kvalobs::kvOperator>& operatorList)
{
  TRY_KVALOBS(getKvOperator, (operatorList));
}
