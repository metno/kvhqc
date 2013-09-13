#include "common/test/FakeKvApp.hh"
#include "common/test/TestHelpers.hh"
#include "common/KvalobsAccess.hh"
inline TimeRange t_32780_20121207()
    { return TimeRange(s2t("2012-12-01 06:00:00"), s2t("2012-12-07 06:00:00")); }
#ifdef LOAD_DECL_ONLY
void load_32780_20121207(FakeKvApp& fa);
#else // LOAD_DECL_ONLY
void load_32780_20121207(FakeKvApp& fa)
{
    fa.insertStation = 32780;
    fa.insertParam = 18;
    fa.insertType = 302;
    fa.insertData("2012-12-01 06:00:00",       1.0,       1.0, "0110000000000000", "");
    fa.insertData("2012-12-02 06:00:00",       1.0,       1.0, "0110000000000000", "");
    fa.insertData("2012-12-03 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-04 06:00:00",       3.0,       3.0, "0110000000000000", "");
    fa.insertData("2012-12-05 06:00:00",       3.0,       3.0, "0110000000000000", "");
    fa.insertData("2012-12-06 06:00:00",       3.0,       3.0, "0110000000000000", "");
    fa.insertData("2012-12-07 06:00:00",       3.0,       3.0, "0110000000000000", "");
    fa.insertParam = 34;
    fa.insertData("2012-12-02 12:00:00",       5.0,       5.0, "0140000000000000", "QC1-2-72.b11");
    fa.insertData("2012-12-02 18:00:00",       2.0,       2.0, "0140000000000000", "QC1-2-72.b11");
    fa.insertData("2012-12-03 12:00:00",       2.0,       2.0, "0100000000000000", "");
    fa.insertData("2012-12-03 18:00:00",       2.0,       2.0, "0100000000000000", "");
    fa.insertData("2012-12-05 06:00:00",       5.0,       5.0, "0100000000100000", "");
    fa.insertParam = 35;
    fa.insertData("2012-12-02 12:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-12-02 18:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-12-03 12:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-12-03 18:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-12-05 06:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertParam = 110;
    fa.insertData("2012-12-01 06:00:00",      -1.0,      -1.0, "0110000000001000", "");
    fa.insertData("2012-12-02 06:00:00",      -1.0,      -1.0, "0110000000001000", "");
    fa.insertData("2012-12-03 06:00:00",  -32767.0,  -32767.0, "0000003000003000", "QC1-7-110");
    fa.insertData("2012-12-04 06:00:00",       0.1,       0.1, "0110000000003000", "QC1-7-110");
    fa.insertData("2012-12-05 06:00:00",       0.1,       0.1, "0110000000001000", "");
    fa.insertData("2012-12-06 06:00:00",      -1.0,      -1.0, "0110000000001000", "");
    fa.insertData("2012-12-07 06:00:00",      -1.0,      -1.0, "0110000000001000", "");
    fa.insertParam = 112;
    fa.insertData("2012-12-01 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-12-02 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-12-03 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-04 06:00:00",       2.0,       2.0, "0110000000100000", "");
    fa.insertData("2012-12-05 06:00:00",       2.0,       2.0, "0110000000100000", "");
    fa.insertData("2012-12-06 06:00:00",       2.0,       2.0, "0110000000100000", "");
    fa.insertData("2012-12-07 06:00:00",       2.0,       2.0, "0110000000000000", "");

    fa.insertStation = 32780;
    fa.insertParam = 110;
    fa.insertModel("2012-12-01 06:00:00",       0.0);
    fa.insertModel("2012-12-01 18:00:00",       0.2);
    fa.insertModel("2012-12-02 06:00:00",       0.1);
    fa.insertModel("2012-12-02 18:00:00",       0.0);
    fa.insertModel("2012-12-03 06:00:00",       4.4);
    fa.insertModel("2012-12-03 18:00:00",       1.4);
    fa.insertModel("2012-12-04 06:00:00",       0.6);
    fa.insertModel("2012-12-04 18:00:00",       0.0);
    fa.insertModel("2012-12-05 06:00:00",       0.0);
    fa.insertModel("2012-12-05 18:00:00",       0.7);
    fa.insertModel("2012-12-06 06:00:00",       0.2);
    fa.insertModel("2012-12-06 18:00:00",       0.0);
    fa.insertModel("2012-12-07 06:00:00",       0.0);

    fa.mKvStations.push_back(kvalobs::kvStation(32780, 59.144400, 9.266800, 113.000000, 0.0f, "HØIDALEN I SOLUM", 0, 0, "?", "?", "?", 9, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1897-06-01 00:00:00"))));

    fa.mObsPgm.push_back(kvalobs::kvObsPgm(32780, 110, 0, 1, 302, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-06-11 00:00:00")),
                                           timeutil::to_miTime(timeutil::ptime())));

    {
        const TimeRange t = t_32780_20121207();
        fa.kda->addSubscription(ObsSubscription(32780, t));
    }
}
#endif // !LOAD_DECL_ONLY
