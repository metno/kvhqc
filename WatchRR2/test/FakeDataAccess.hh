
#ifndef FakeDataAccess_hh
#define FakeDataAccess_hh 1

#include "KvBufferedAccess.hh"
#include "TestHelpers.hh"

#include <kvalobs/kvData.h>

class FakeDataAccess : public KvBufferedAccess {
public:

    int insertStation;
    int insertParam;
    int insertType;

    void insert(int stationId, int paramId, int typeId, const std::string& obstime, float orig, float corr,
                const std::string& controlinfo="0000000000000000", const std::string& cfailed="");

    void insert(const std::string& obstime, float orig, float corr,
                const std::string& controlinfo="0000000000000000", const std::string& cfailed="")
        { insert(insertStation, insertParam, insertType, obstime, orig, corr, controlinfo, cfailed); }

    void insert(const std::string& obstime, float orig_corr,
                const std::string& controlinfo="0000000000000000", const std::string& cfailed="")
        { insert(insertStation, insertParam, insertType, obstime, orig_corr, orig_corr, controlinfo, cfailed); }
    
    bool erase(ObsDataPtr obs);

protected:
    virtual bool isSubscribed(const SensorTime& st);
};

typedef boost::shared_ptr<FakeDataAccess> FakeDataAccessPtr;

#endif // FakeDataAccess_hh
