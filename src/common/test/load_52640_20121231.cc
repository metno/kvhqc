#include "common/test/FakeKvApp.hh"
#include "common/test/TestHelpers.hh"
#include "common/KvalobsAccess.hh"
inline TimeSpan t_52640_20121231()
    { return TimeSpan(s2t("2012-12-01 06:00"), s2t("2012-12-31 06:00")); }
#ifdef LOAD_DECL_ONLY
void load_52640_20121231(FakeKvApp& fa);
#else // LOAD_DECL_ONLY
void load_52640_20121231(FakeKvApp& fa)
{
    fa.insertStation = 52640;
    fa.insertParam = 18;
    fa.insertType = 402;
    fa.insertData("2012-12-01 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-02 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-03 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-04 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-05 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-06 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-07 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-08 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-09 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-10 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-11 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-12 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-13 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-14 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-15 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-16 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-17 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-18 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-22 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-23 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-24 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-25 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-26 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-27 06:00:00",       4.0,       4.0, "0110000000000010", "");
    fa.insertData("2012-12-28 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertParam = 34;
    fa.insertData("2012-12-24 18:00:00",       2.0,       2.0, "0140000000000010", "QC1-2-72.c11");
    fa.insertData("2012-12-25 06:00:00",       2.0,       2.0, "0140000000000010", "QC1-2-72.c11");
    fa.insertData("2012-12-25 12:00:00",       7.0,       7.0, "0100000000000010", "");
    fa.insertData("2012-12-25 18:00:00",       3.0,       3.0, "0140000000000010", "QC1-2-72.c11");
    fa.insertData("2012-12-26 06:00:00",       7.0,       7.0, "0140000000000010", "QC1-2-72.c11");
    fa.insertData("2012-12-26 12:00:00",       7.0,       7.0, "0100000000000010", "");
    fa.insertData("2012-12-26 18:00:00",       4.0,       4.0, "0100000000000010", "");
    fa.insertData("2012-12-27 06:00:00",       2.0,       2.0, "0100000000000010", "");
    fa.insertData("2012-12-27 12:00:00",       5.0,       5.0, "0140000000000010", "QC1-2-72.c11");
    fa.insertData("2012-12-27 18:00:00",       5.0,       5.0, "0140000000000010", "QC1-2-72.c11");
    fa.insertData("2012-12-28 06:00:00",       5.0,       5.0, "0140000000000010", "QC1-2-72.c11");
    fa.insertParam = 35;
    fa.insertData("2012-12-24 18:00:00",       0.0,       0.0, "0100000000000010", "");
    fa.insertData("2012-12-25 06:00:00",       1.0,       1.0, "0100000000000010", "");
    fa.insertData("2012-12-25 12:00:00",       1.0,       1.0, "0100000000000010", "");
    fa.insertData("2012-12-25 18:00:00",       1.0,       1.0, "0100000000000010", "");
    fa.insertData("2012-12-26 06:00:00",       1.0,       1.0, "0100000000000010", "");
    fa.insertData("2012-12-26 12:00:00",       1.0,       1.0, "0100000000000010", "");
    fa.insertData("2012-12-26 18:00:00",       1.0,       1.0, "0100000000000010", "");
    fa.insertData("2012-12-27 06:00:00",       1.0,       1.0, "0100000000000010", "");
    fa.insertData("2012-12-27 12:00:00",       1.0,       1.0, "0100000000000010", "");
    fa.insertData("2012-12-27 18:00:00",       1.0,       1.0, "0100000000000010", "");
    fa.insertData("2012-12-28 06:00:00",       1.0,       1.0, "0100000000000010", "");
    fa.insertParam = 110;
    fa.insertData("2012-12-01 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-02 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-03 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-04 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-05 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-06 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-07 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-08 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-09 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-10 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-11 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-12 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-13 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-14 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-15 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-16 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-17 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-18 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-22 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "");
    fa.insertData("2012-12-23 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "");
    fa.insertData("2012-12-24 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "");
    fa.insertData("2012-12-25 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "");
    fa.insertData("2012-12-26 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "QC1-7-110b");
    fa.insertData("2012-12-27 06:00:00",      45.0,      45.0, "0110004000004000", "QC1-7-110b");
    fa.insertData("2012-12-28 06:00:00",  -32767.0,  -32767.0, "0000003000002000", "");
    fa.insertParam = 112;
    fa.insertData("2012-12-01 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-02 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-03 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-04 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-05 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-06 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-07 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-08 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-09 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-10 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-11 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-12 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-13 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-14 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-15 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-16 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-17 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-18 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-22 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-23 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-24 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-25 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-26 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");
    fa.insertData("2012-12-27 06:00:00",      10.0,      10.0, "0110000000000010", "");
    fa.insertData("2012-12-28 06:00:00",  -32767.0,  -32767.0, "0000003000000000", "");

    fa.insertStation = 52640;
    fa.insertParam = 110;
    fa.insertModel("2012-12-01 06:00:00",       0.5);
    fa.insertModel("2012-12-01 18:00:00",       0.4);
    fa.insertModel("2012-12-02 06:00:00",       0.2);
    fa.insertModel("2012-12-02 18:00:00",       3.2);
    fa.insertModel("2012-12-03 06:00:00",       0.1);
    fa.insertModel("2012-12-03 18:00:00",       0.0);
    fa.insertModel("2012-12-04 06:00:00",       0.0);
    fa.insertModel("2012-12-04 18:00:00",       0.0);
    fa.insertModel("2012-12-05 06:00:00",       0.0);
    fa.insertModel("2012-12-05 18:00:00",       0.0);
    fa.insertModel("2012-12-06 06:00:00",       0.0);
    fa.insertModel("2012-12-06 18:00:00",       0.0);
    fa.insertModel("2012-12-07 06:00:00",       0.1);
    fa.insertModel("2012-12-07 18:00:00",       0.0);
    fa.insertModel("2012-12-08 06:00:00",       0.0);
    fa.insertModel("2012-12-08 18:00:00",       0.0);
    fa.insertModel("2012-12-09 06:00:00",       5.8);
    fa.insertModel("2012-12-09 18:00:00",      16.0);
    fa.insertModel("2012-12-10 06:00:00",       0.2);
    fa.insertModel("2012-12-11 06:00:00",       0.0);
    fa.insertModel("2012-12-11 18:00:00",       0.9);
    fa.insertModel("2012-12-12 06:00:00",       1.4);
    fa.insertModel("2012-12-12 18:00:00",       0.0);
    fa.insertModel("2012-12-13 06:00:00",       0.0);
    fa.insertModel("2012-12-13 18:00:00",       1.7);
    fa.insertModel("2012-12-14 06:00:00",       1.0);
    fa.insertModel("2012-12-14 18:00:00",       0.9);
    fa.insertModel("2012-12-15 06:00:00",       0.1);
    fa.insertModel("2012-12-15 18:00:00",       0.0);
    fa.insertModel("2012-12-16 06:00:00",       1.0);
    fa.insertModel("2012-12-16 18:00:00",       1.1);
    fa.insertModel("2012-12-17 06:00:00",       4.4);
    fa.insertModel("2012-12-17 18:00:00",       1.2);
    fa.insertModel("2012-12-18 06:00:00",       0.1);
    fa.insertModel("2012-12-18 18:00:00",       0.0);
    fa.insertModel("2012-12-19 06:00:00",       0.0);
    fa.insertModel("2012-12-19 18:00:00",       0.0);
    fa.insertModel("2012-12-20 06:00:00",       0.1);
    fa.insertModel("2012-12-20 18:00:00",       0.0);
    fa.insertModel("2012-12-21 06:00:00",       0.0);
    fa.insertModel("2012-12-21 18:00:00",       0.0);
    fa.insertModel("2012-12-22 06:00:00",       0.0);
    fa.insertModel("2012-12-22 18:00:00",       0.0);
    fa.insertModel("2012-12-23 06:00:00",       0.0);
    fa.insertModel("2012-12-23 18:00:00",       0.0);
    fa.insertModel("2012-12-24 06:00:00",       0.0);
    fa.insertModel("2012-12-24 18:00:00",       5.8);
    fa.insertModel("2012-12-25 06:00:00",      27.2);
    fa.insertModel("2012-12-25 18:00:00",      11.8);
    fa.insertModel("2012-12-26 06:00:00",      28.8);
    fa.insertModel("2012-12-26 18:00:00",      37.9);
    fa.insertModel("2012-12-27 06:00:00",      18.3);
    fa.insertModel("2012-12-27 18:00:00",       5.1);
    fa.insertModel("2012-12-28 06:00:00",       3.1);
    fa.insertModel("2012-12-28 18:00:00",       0.8);
    fa.insertModel("2012-12-29 06:00:00",      22.7);
    fa.insertModel("2012-12-29 18:00:00",     104.2);
    fa.insertModel("2012-12-30 06:00:00",     102.4);
    fa.insertModel("2012-12-30 18:00:00",      28.4);
    fa.insertModel("2012-12-31 06:00:00",      29.9);

    fa.addStation(kvalobs::kvStation(52640, 60.874000, 5.593700, 7.000000, 0.0f, "MATRE KRAFTSTASJON", 0, 0, "?", "?", "?", 10, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1975-07-01 00:00:00"))));

    fa.addObsPgm(kvalobs::kvObsPgm(52640, 110, 0, 1, 402, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2004-12-24 00:00:00")), timeutil::to_miTime(timeutil::ptime())));
}
#endif // !LOAD_DECL_ONLY
