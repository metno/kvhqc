
#ifndef COMMON_KVALOBSREINSERTER_HH
#define COMMON_KVALOBSREINSERTER_HH 1

#include "AbstractReinserter.hh"

#include <decodeutility/DataReinserter.h>
#include <kvcpp/KvApp.h>
#include <memory>

class KvalobsReinserter : public AbstractReinserter, private boost::noncopyable {
public:
  typedef kvalobs::DataReinserter<kvservice::KvApp> Reinserter_t;

  KvalobsReinserter(std::shared_ptr<kvservice::KvApp> app);

  bool authenticate();
  bool authenticated() const
    { return mDataReinserter.get() != 0; }

  virtual bool storeChanges(const kvData_l& toUpdate, const kvData_l& toInsert);

private:
  std::shared_ptr<kvservice::KvApp> mApp;
  std::unique_ptr<Reinserter_t> mDataReinserter;
};

#endif // COMMON_KVALOBSREINSERTER_HH
