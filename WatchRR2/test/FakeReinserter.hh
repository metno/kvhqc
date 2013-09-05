
#ifndef FakeReinserter_hh
#define FakeReinserter_hh 1

#include "hqcdefs.h"

class FakeReinserter : public HqcReinserter
{
public:
    FakeReinserter();
    ~FakeReinserter();

    virtual const CKvalObs::CDataSource::Result_var insert(kvalobs::kvData &d) const;
    virtual const CKvalObs::CDataSource::Result_var insert(std::list<kvalobs::kvData> &dl) const;
    virtual const CKvalObs::CDataSource::Result_var insert(const kvalobs::serialize::KvalobsData & data) const;
};


#endif // FakeReinserter_hh
