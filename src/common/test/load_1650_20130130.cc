#include "common/test/FakeKvApp.hh"
#include "common/KvalobsAccess.hh"
inline TimeRange t_1650_20130130()
    { return TimeRange(timeutil::from_iso_extended_string("2013-01-01 06:00:00"), timeutil::from_iso_extended_string("2013-01-30 06:00:00")); }
#ifdef LOAD_DECL_ONLY
void load_1650_20130130(FakeKvApp& fa);
#else // LOAD_DECL_ONLY
void load_1650_20130130(FakeKvApp& fa)
{
    fa.insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_1650_20130130.txt");
    fa.insertModelFromFile(std::string(TEST_SOURCE_DIR)+"/model_1650_20130130.txt");

    fa.mKvStations.push_back(kvalobs::kvStation(3810, 59.587200, 11.166000, 141.000000, 0.0f, "ASKIM II", 0, 0, "?", "?", "?", 9, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1999-05-27 00:00:00"))));
    fa.mKvStations.push_back(kvalobs::kvStation(1650, 59.300600, 11.659900, 113.000000, 0.0f, "STRØMSFOSS SLUSE", 0, 0, "?", "?", "?", 9, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1883-01-01 00:00:00"))));
    fa.mKvStations.push_back(kvalobs::kvStation(17500, 59.496300, 11.013300, 131.000000, 0.0f, "FLØTER", 0, 0, "?", "?", "?", 9, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1971-11-01 00:00:00"))));
    fa.mKvStations.push_back(kvalobs::kvStation(3290, 59.386300, 11.387500, 100.000000, 0.0f, "RAKKESTAD", 0, 0, "?", "?", "?", 8, true, timeutil::to_miTime(timeutil::from_iso_extended_string("2009-07-02 00:00:00"))));
    fa.mKvStations.push_back(kvalobs::kvStation(1080, 59.036700, 11.044400, 17.000000, 0.0f, "HVALER", 0, 0, "?", "?", "?", 9, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1908-11-01 00:00:00"))));
    fa.mKvStations.push_back(kvalobs::kvStation(1400, 59.147700, 11.558300, 114.000000, 0.0f, "BREKKE SLUSE", 0, 0, "?", "?", "?", 10, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1965-06-01 00:00:00"))));
    fa.mKvStations.push_back(kvalobs::kvStation(3190, 59.285700, 11.114800, 57.000000, 0.0f, "SARPSBORG", 0, 0, "?", "?", "?", 8, true, timeutil::to_miTime(timeutil::from_iso_extended_string("2002-03-10 00:00:00"))));

    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1080, 34, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1080, 35, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1080, 36, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1080, 37, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1080, 38, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1080, 39, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1650, 34, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1650, 35, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1650, 36, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1650, 37, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1650, 38, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1650, 39, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(17500, 34, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(17500, 35, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(17500, 36, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(17500, 37, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(17500, 38, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(17500, 39, 0, 1, 402, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(17500, 112, 0, 1, 402, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1080, 18, 0, 1, 402, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1080, 110, 0, 1, 402, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1080, 112, 0, 1, 402, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1650, 18, 0, 1, 402, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1650, 110, 0, 1, 402, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1650, 112, 0, 1, 402, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(17500, 18, 0, 1, 402, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(17500, 110, 0, 1, 402, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(3810, 105, 0, 1, 404, 0,
                                           1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("1999-05-27 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1400, 18, 0, 1, 302, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-10 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1400, 34, 0, 1, 302, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-10 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1400, 35, 0, 1, 302, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-10 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1400, 36, 0, 1, 302, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-10 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1400, 37, 0, 1, 302, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-10 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1400, 38, 0, 1, 302, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-10 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1400, 39, 0, 1, 302, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-10 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1400, 110, 0, 1, 302, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-10 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(1400, 112, 0, 1, 302, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-10 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(3190, 109, 0, 1, 308, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-13 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(3190, 108, 0, 1, 308, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-13 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(3190, 110, 0, 1, 308, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-13 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
    fa.mObsPgm.push_back(kvalobs::kvObsPgm(3190, 112, 0, 1, 308, 1,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2012-11-13 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
}
#endif // !LOAD_DECL_ONLY

// generated by '/home/alexanderb/tools/kvhqc/fetch_test_data.py' '1650,1400,3290,3190,3810,17500,1080' '18,34,35,36,37,38,39,110,112' '2013-01-01 06:00:00' '2013-01-30 06:00:00' on 2013-02-06T11:12:32.965002
// Local Variables:
// buffer-read-only: nil
// End:
