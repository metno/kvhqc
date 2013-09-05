
#ifndef FakeReinserter_hh
#define FakeReinserter_hh 1

#include "HqcDataReinserter.h"

class FakeReinserter : public HqcReinserter
{
public:
    FakeReinserter();
    ~FakeReinserter();

    void setInsertSuccess(bool successful);

    virtual const CKvalObs::CDataSource::Result_var insert(kvalobs::kvData &d) const;
    virtual const CKvalObs::CDataSource::Result_var insert(std::list<kvalobs::kvData> &dl) const;
    virtual const CKvalObs::CDataSource::Result_var insert(const kvalobs::serialize::KvalobsData & data) const;

private:
    const CKvalObs::CDataSource::Result_var makeInsertResult() const;

private:
    bool mInsertSuccess;
};


#endif // FakeReinserter_hh
