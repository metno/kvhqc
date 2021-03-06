
#ifndef FAKEAPP_HH
#define FAKEAPP_HH 1

#include "common/KvalobsAccess.hh"
#include "common/Sensor.hh"

#include <decodeutility/DataReinserter.h>
#include <kvcpp/KvApp.h>

namespace kvservice {
class KvGetDataReceiver;
class WhichDataHelper;
}

class FakeKvApp : public kvservice::KvApp
{
public:
    FakeKvApp();
    virtual ~FakeKvApp();

public:
    int insertStation;
    int insertParam;
    int insertType;

    void insertDataFromFile(const std::string& filename);

    void insertData(int stationId, int paramId, int typeId, const std::string& obstime, float orig, float corr,
                    const std::string& controlinfo="0000000000000000", const std::string& cfailed="");

    void insertData(const std::string& obstime, float orig, float corr,
                    const std::string& controlinfo="0000000000000000", const std::string& cfailed="")
        { insertData(insertStation, insertParam, insertType, obstime, orig, corr, controlinfo, cfailed); }
    
    void insertData(const std::string& obstime, float orig_corr,
                    const std::string& controlinfo="0000000000000000", const std::string& cfailed="")
        { insertData(insertStation, insertParam, insertType, obstime, orig_corr, orig_corr, controlinfo, cfailed); }
    
    bool eraseData(const SensorTime& st);

    void insertModelFromFile(const std::string& filename);

    void insertModel(int stationId, int paramId, const std::string& obstime, float value);

    void insertModel(const std::string& obstime, float value)
        { insertModel(insertStation, insertParam, obstime, value); }

    void addStation(const std::string& line);
    void addObsPgm(const std::string& line);

    bool eraseModel(const SensorTime& st);

#ifndef KVALOBS_USE_BOOST_DATE_TIME
    typedef miutil::miTime kvtime_t;
#else // ! KVALOBS_USE_BOOST_DATE_TIME
    typedef timeutil::ptime kvtime_t;
#endif // KVALOBS_USE_BOOST_DATE_TIME

    virtual bool getKvData(kvservice::KvGetDataReceiver &dataReceiver, const kvservice::WhichDataHelper &wd);
    virtual bool getKvRejectDecode(const CKvalObs::CService::RejectDecodeInfo &decodeInfo, kvservice::RejectDecodeIterator &it);
    virtual bool getKvParams(std::list<kvalobs::kvParam> &paramList);
    virtual bool getKvStations(std::list<kvalobs::kvStation> &stationList);
    virtual bool getKvModelData(std::list<kvalobs::kvModelData> &dataList, const kvservice::WhichDataHelper &wd);
    virtual bool getKvReferenceStations( int stationid, int paramid,std::list<kvalobs::kvReferenceStation> &refList);
    virtual bool getKvTypes(std::list<kvalobs::kvTypes> &typeList);
    virtual bool getKvOperator(std::list<kvalobs::kvOperator> &operatorList);
    virtual bool getKvStationParam(std::list<kvalobs::kvStationParam> &stParam, int stationid, int paramid = -1, int day = -1);
    virtual bool getKvStationMetaData(std::list<kvalobs::kvStationMetadata> &stMeta,
                                       int stationid, const kvtime_t &obstime,
                                       const std::string & metadataName = "");
    virtual bool getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm, const std::list<long> &stationList, bool aUnion);
    virtual bool getKvData(kvservice::KvObsDataList &dataList, const kvservice::WhichDataHelper &wd);
    virtual bool getKvWorkstatistik(CKvalObs::CService::WorkstatistikTimeType timeType,
                                    const kvtime_t &from, const kvtime_t &to,
                                    kvservice::WorkstatistikIterator &it);


    virtual const CKvalObs::CDataSource::Result_var sendDataToKv(const char *data, const char *obsType);
    
    virtual kvservice::KvApp::SubscriberID subscribeDataNotify(const kvservice::KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que);
    virtual kvservice::KvApp::SubscriberID subscribeData(const kvservice::KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que);
    virtual kvservice::KvApp::SubscriberID subscribeKvHint(dnmi::thread::CommandQue &que);  
    virtual void unsubscribe(const kvservice::KvApp::SubscriberID &subscriberid);
    virtual void unsubscribeAll();
    
    virtual bool shutdown() const { return false; }
    virtual void doShutdown() { }
    virtual void run() { }

public:
    std::list<kvalobs::kvParam>  mKvParams;
    std::list<kvalobs::kvStation> mKvStations;
    std::list<kvalobs::kvObsPgm> mObsPgm;
    KvalobsAccessPtr kda;

private:
    typedef std::map<SensorTime, kvalobs::kvData, lt_SensorTime> Data_t;
    Data_t mData;

    typedef std::map<SensorTime, kvalobs::kvModelData, lt_SensorTime> ModelData_t;
    ModelData_t mModelData;
};

#endif /* FAKEAPP_HH */
