
#ifndef KvServiceHelper_hh
#define KvServiceHelper_hh 1

#include <boost/signals.hpp>

class TimeRange;
namespace kvalobs {
class kvModelData;
class kvRejectdecode;
class kvParam;
class kvStation;
class kvTypes;
class kvObsPgm;
class kvOperator;
}
namespace kvservice {
class KvGetDataReceiver;
class WhichDataHelper;
}

class KvServiceHelper
{
public:
  KvServiceHelper();
  ~KvServiceHelper();

  /** Query last known availability of kvServiced. Does not re-check. */
  bool isKvalobsAvailable() const
    { return mKvalobsAvailable; }

  /** Query last known availability of kvServiced. Does not re-check. */
  bool checkKvalobsAvailability();

  bool getKvData(kvservice::KvGetDataReceiver& dataReceiver, const kvservice::WhichDataHelper& wd);
  bool getKvModelData(std::list<kvalobs::kvModelData> &dataList, const kvservice::WhichDataHelper& wd);
  bool getKvRejectDecode(std::list<kvalobs::kvRejectdecode>& rejectList, const TimeRange& timeLimits);
  bool getKvParams(std::list<kvalobs::kvParam>& paramList);
  bool getKvStations( std::list<kvalobs::kvStation>& stationList);
  bool getKvTypes(std::list<kvalobs::kvTypes>& typeList);
  bool getKvObsPgm(std::list<kvalobs::kvObsPgm>& obsPgm, const std::list<long>& stationList);
  bool getKvOperator(std::list<kvalobs::kvOperator>& operatorList );

  boost::signal1<void, bool> kvalobsAvailable;

  static KvServiceHelper* instance()
    { return sInstance; }

private:
  bool updateKvalobsAvailability(bool available);

private:
  bool mKvalobsAvailable;

  static KvServiceHelper* sInstance;
};

#endif // KvServiceHelper_hh
