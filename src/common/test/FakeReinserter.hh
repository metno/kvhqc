
#ifndef FakeReinserter_hh
#define FakeReinserter_hh 1

#include "common/AbstractReinserter.hh"

class FakeReinserter : public AbstractReinserter
{
public:
  FakeReinserter();
  void setInsertSuccess(bool successful);
  virtual bool storeChanges(const kvData_l& toUpdate, const kvData_l& toInsert);

private:
  bool mInsertSuccess;
};

#endif // FakeReinserter_hh
