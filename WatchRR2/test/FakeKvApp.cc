
#include "FakeKvApp.hh"

#include "Helpers.hh"
#include "KvalobsAccess.hh"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define NDEBUG
#include "debug.hh"

class FakeReinserter : public kvalobs::DataReinserter<kvservice::KvApp>
{
public:
    FakeReinserter();
    ~FakeReinserter();

    virtual const CKvalObs::CDataSource::Result_var insert(kvalobs::kvData &d) const;
    virtual const CKvalObs::CDataSource::Result_var insert(std::list<kvalobs::kvData> &dl) const;
    virtual const CKvalObs::CDataSource::Result_var insert(const kvalobs::serialize::KvalobsData & data) const;
};

// ========================================================================

FakeKvApp::FakeKvApp()
    : mFakeReinserter(new FakeReinserter)
{
    kvservice::KvApp::kvApp = this;
    kda = boost::make_shared<KvalobsAccess>();
    kda->setReinserter(mFakeReinserter);
}

FakeKvApp::~FakeKvApp()
{
    delete mFakeReinserter;
    kvservice::KvApp::kvApp = 0;
}

void FakeKvApp::insertData(int stationId, int paramId, int typeId, const std::string& obstime, float orig, float corr,
                           const std::string& controlinfo, const std::string& cfailed)
{
    const kvalobs::kvData data(stationId, timeutil::to_miTime(timeutil::from_iso_extended_string(obstime)), orig,
                               paramId, timeutil::to_miTime(timeutil::ptime()), typeId, 0, 0, corr,
                               controlinfo, kvalobs::kvUseInfo(), cfailed);
    const SensorTime st(Helpers::sensorFromKvData(data), timeutil::from_iso_extended_string(obstime));
    const Data_t::iterator it = mData.find(st);
    if (it != mData.end())
        mData.erase(it);
    mData.insert(Data_t::value_type(st, data));
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

void FakeKvApp::registerStation(int id, float lon, float lat, const std::string& name)
{
    const timeutil::ptime t = timeutil::from_iso_extended_string("1700-01-01 00:00:00");
    mKvStations.push_back(kvalobs::kvStation(id, lon, lat, 0.0f, 0.0f, name, 0, 0, "?", "?", "?", 0, true,
                                             timeutil::to_miTime(t)));
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
    paramList.clear();
    return true;
}

bool FakeKvApp::getKvStations(std::list<kvalobs::kvStation> &stationList)
{
    stationList = mKvStations;
    return true;
}

bool FakeKvApp::getKvModelData(std::list<kvalobs::kvModelData> &dataList, const kvservice::WhichDataHelper &wd )
{
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
            if (fromTime <= ot and ((not hasToTime) or (ot <= toTime)))
                dataList.push_back(data);
        }
    }
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
    dataList.clear();

    const CKvalObs::CService::WhichDataList& whichData = *wd.whichData();
    for (long wi = 0; wi < whichData.length(); ++wi) {
        const long sid = whichData[wi].stationid;
        const timeutil::ptime fromTime = timeutil::from_iso_extended_string(std::string(whichData[wi].fromObsTime));
        const timeutil::ptime toTime   = timeutil::from_iso_extended_string(std::string(whichData[wi].toObsTime));
        const bool hasToTime = (not toTime.is_not_a_date_time());
        DBG(DBG1(sid) << DBG1(fromTime) << DBG1(toTime) << DBG1(hasToTime));

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
            DBGV(data);
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

FakeReinserter::FakeReinserter()
    : kvalobs::DataReinserter<kvservice::KvApp>(0, 123)
{
}

FakeReinserter::~FakeReinserter()
{
}

const CKvalObs::CDataSource::Result_var FakeReinserter::insert(kvalobs::kvData&) const
{
    return makeResult(CKvalObs::CDataSource::OK);
}

const CKvalObs::CDataSource::Result_var FakeReinserter::insert(std::list<kvalobs::kvData>&) const
{
    return makeResult(CKvalObs::CDataSource::OK);
}

const CKvalObs::CDataSource::Result_var FakeReinserter::insert(const kvalobs::serialize::KvalobsData&) const
{
    return makeResult(CKvalObs::CDataSource::OK);
}
