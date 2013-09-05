
#include "FakeReinserter.hh"

static const CKvalObs::CDataSource::Result_var makeResult(CKvalObs::CDataSource::EResult what)
{
    CKvalObs::CDataSource::Result_var ret(new CKvalObs::CDataSource::Result);
    ret->res = what;
    ret->message = "FakeKvApp response";
    return ret;
}

// ========================================================================
    
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
