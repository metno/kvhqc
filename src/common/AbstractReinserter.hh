
#ifndef COMMON_ABSTRACTREINSERTER_HH
#define COMMON_ABSTRACTREINSERTER_HH 1

#include "util/boostutil.hh"

#include <kvalobs/kvData.h>
#include <list>

class AbstractReinserter {
public:
  typedef std::list<kvalobs::kvData> kvData_l;

  virtual ~AbstractReinserter() { }
  virtual bool storeChanges(const kvData_l& toUpdate, const kvData_l& toInsert) = 0;
};

HQC_TYPEDEF_P(AbstractReinserter);

#endif // COMMON_ABSTRACTREINSERTER_HH
