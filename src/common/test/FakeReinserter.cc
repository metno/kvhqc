
#include "FakeReinserter.hh"

FakeReinserter::FakeReinserter()
  : mInsertSuccess(true)
{
}

void FakeReinserter::setInsertSuccess(bool successful)
{
  mInsertSuccess = successful;
}

bool FakeReinserter::storeChanges(const kvData_l&, const kvData_l&)
{
  return mInsertSuccess;
}
