
#include "FakeDataAccess.hh"

#include "Helpers.hh"
#include "timeutil.hh"
#include <boost/make_shared.hpp>

void FakeDataAccess::insert(int stationId, int paramId, int typeId, const std::string& obstime, float orig, float corr,
                            const std::string& controlinfo, const std::string& cfailed)
{
    const kvalobs::kvData data(stationId, timeutil::to_miTime(timeutil::from_iso_extended_string(obstime)), orig,
                               paramId, timeutil::to_miTime(timeutil::ptime()), typeId, 0, 0, corr,
                               controlinfo, kvalobs::kvUseInfo(), cfailed);
    receive(data);
}

bool FakeDataAccess::erase(ObsDataPtr obs)
{
    return drop(obs->sensorTime());
}

bool FakeDataAccess::isSubscribed(const SensorTime& st)
{
    if (mSubscriptions.empty())
        return true;
    else
        return KvBufferedAccess::isSubscribed(st);
}

// ========================================================================

::testing::AssertionResult AssertCorrControl(const char* ec_expr, const char* eci_expr, const char* a_expr,
                                             float ec, const std::string& eci, const ObsDataPtr& a)
{
    ::testing::Message msg;
    if (not a) {
        msg << "(no obs " << a_expr << ")";
        return ::testing::AssertionFailure(msg);
    }
    bool failed = false;
    if (not Helpers::float_eq()(ec, a->corrected())) {
        msg << "(corrected ex:" << ec_expr << " != ac:" << a->corrected() << ")";
        failed = true;
    }
    if( eci != a->controlinfo().flagstring() ) {
        if( failed )
            msg << "; ";
        msg << "(controlinfo ex:" << eci_expr << " != ac:" << a->controlinfo().flagstring() << ")";
        failed = true;
    }
    return failed ? ::testing::AssertionFailure(msg) : ::testing::AssertionSuccess();
}
