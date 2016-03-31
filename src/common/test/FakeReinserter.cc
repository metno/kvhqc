
#include "FakeReinserter.hh"

FakeReinserter::FakeReinserter()
    : mInsertSuccess(true)
{
}

FakeReinserter::~FakeReinserter()
{
}

void FakeReinserter::setInsertSuccess(bool successful)
{
    mInsertSuccess = successful;
}

bool FakeReinserter::insert(std::list<kvalobs::kvData>&) const
{
    return mInsertSuccess;
}
