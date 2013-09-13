
#include "FakeReinserter.hh"

FakeReinserter::FakeReinserter()
    : kvalobs::DataReinserter<kvservice::KvApp>(0, 123)
    , mInsertSuccess(true)
{
}

FakeReinserter::~FakeReinserter()
{
}

void FakeReinserter::setInsertSuccess(bool successful)
{
    mInsertSuccess = successful;
}

const CKvalObs::CDataSource::Result_var FakeReinserter::insert(kvalobs::kvData&) const
{
    return makeInsertResult();
}

const CKvalObs::CDataSource::Result_var FakeReinserter::insert(std::list<kvalobs::kvData>&) const
{
    return makeInsertResult();
}

const CKvalObs::CDataSource::Result_var FakeReinserter::insert(const kvalobs::serialize::KvalobsData&) const
{
    return makeInsertResult();
}

const CKvalObs::CDataSource::Result_var FakeReinserter::makeInsertResult() const
{
    using namespace CKvalObs::CDataSource;
    Result_var ret(new Result);
    ret->res = (mInsertSuccess ? OK : ERROR);
    ret->message = "FakeKvApp insert result";
    return ret;
}
