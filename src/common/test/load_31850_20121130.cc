#include "common/test/FakeKvApp.hh"
#include "common/test/TestHelpers.hh"
#include "common/KvalobsAccess.hh"
inline TimeRange t_31850_20121130()
    { return TimeRange(s2t("2012-10-20 06:00:00"), s2t("2012-11-30 06:00:00")); }
#ifdef LOAD_DECL_ONLY
void load_31850_20121130(FakeKvApp& fa);
#else // LOAD_DECL_ONLY
void load_31850_20121130(FakeKvApp& fa)
{
    fa.insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_31850_20121130.txt");
    fa.insertModelFromFile(std::string(TEST_SOURCE_DIR)+"/model_31850_20121130.txt");

    fa.mKvStations.push_back(kvalobs::kvStation(32350, 59.647800, 8.376800, 567.000000, 0.0f, "ÅMOTSDAL", 0, 0, "?", "?", "?", 10, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1971-10-01 00:00:00"))));
    fa.mKvStations.push_back(kvalobs::kvStation(32200, 59.455000, 9.037200, 354.000000, 0.0f, "LIFJELL", 0, 0, "?", "?", "?", 9, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1895-07-01 00:00:00"))));
    fa.mKvStations.push_back(kvalobs::kvStation(31850, 59.603300, 8.713300, 162.000000, 0.0f, "HJARTDAL", 0, 0, "?", "?", "?", 10, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1960-08-01 00:00:00"))));
    fa.mKvStations.push_back(kvalobs::kvStation(31900, 59.745300, 8.810000, 464.000000, 0.0f, "TUDDAL", 0, 0, "?", "?", "?", 9, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1895-07-01 00:00:00"))));
    fa.mKvStations.push_back(kvalobs::kvStation(32850, 59.406300, 8.475500, 77.000000, 0.0f, "KVITESEID - MOEN", 0, 0, "?", "?", "?", 9, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1971-10-01 00:00:00"))));
    fa.mKvStations.push_back(kvalobs::kvStation(31410, 59.880000, 8.666300, 258.000000, 0.0f, "RJUKAN", 0, 0, "?", "?", "?", 9, true, timeutil::to_miTime(timeutil::from_iso_extended_string("2003-01-01 00:00:00"))));

    fa.mObsPgm.push_back(kvalobs::kvObsPgm(31410, 110, 0, 1, 302, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")),
                                                               timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(31900, 110, 0, 1, 302, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")),
                                           timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(32350, 110, 0, 1, 302, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")),
                                           timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(32850, 110, 0, 1, 302, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")),
                                           timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(31850, 110, 0, 1, 302, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2009-02-07 00:00:00")),
                                           timeutil::to_miTime(timeutil::ptime())));
}
#endif // !LOAD_DECL_ONLY
