
#ifndef FakeReinserter_hh
#define FakeReinserter_hh 1

#include "common/AbstractReinserter.hh"

class FakeReinserter : public AbstractReinserter
{
public:
  FakeReinserter();
  ~FakeReinserter();

  void setInsertSuccess(bool successful);

  bool insert(std::list<kvalobs::kvData> &dl) const;

private:
  bool mInsertSuccess;
};


#endif // FakeReinserter_hh
