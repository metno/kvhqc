#include "FakeKvApp.hh"
#include "TestHelpers.hh"
#include "KvalobsAccess.hh"
inline TimeRange t_84070_20120930()
    { return TimeRange(s2t("2012-08-30 06:00:00"), s2t("2012-09-30 06:00:00")); }
#ifdef LOAD_DECL_ONLY
void load_84070_20120930(FakeKvApp& fa);
#else // LOAD_DECL_ONLY
void load_84070_20120930(FakeKvApp& fa)
{
    fa.insertStation = 84070;
    fa.insertParam = 18;
    fa.insertType = 302;
    fa.insertData("2012-08-30 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-08-31 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-01 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-02 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-03 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-04 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-05 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-06 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-07 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-08 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-09 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-10 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-11 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-12 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-13 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-14 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-15 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-16 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-17 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-18 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-19 06:00:00",      -1.0,      -1.0, "0130000000000000", "QC1-2-72.a");
    fa.insertData("2012-09-20 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-21 06:00:00",      -1.0,      -1.0, "0000000000000001", "watchRR");
    fa.insertData("2012-09-22 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-23 06:00:00",      -1.0,      -1.0, "0000000000000001", "watchRR");
    fa.insertData("2012-09-24 06:00:00",      -1.0,      -1.0, "0110000000000001", "watchRR");
    fa.insertData("2012-09-25 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-26 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-27 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-28 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-29 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-30 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertParam = 34;
    fa.insertData("2012-08-31 06:00:00",       3.0,       3.0, "0100000000100000", "");
    fa.insertData("2012-08-31 12:00:00",  -32767.0,       3.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-02 12:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-02 18:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-03 12:00:00",       3.0,       3.0, "0100000000000000", "");
    fa.insertData("2012-09-03 18:00:00",       3.0,       3.0, "0100000000000000", "");
    fa.insertData("2012-09-04 06:00:00",       3.0,       3.0, "0100000000100000", "");
    fa.insertData("2012-09-04 12:00:00",       3.0,       3.0, "0100000000000000", "");
    fa.insertData("2012-09-04 18:00:00",       3.0,       3.0, "0100000000000000", "");
    fa.insertData("2012-09-05 06:00:00",       3.0,       3.0, "0100000000100000", "");
    fa.insertData("2012-09-05 12:00:00",       3.0,       3.0, "0100000000000000", "");
    fa.insertData("2012-09-05 18:00:00",       3.0,       3.0, "0100000000000000", "");
    fa.insertData("2012-09-06 06:00:00",       3.0,       3.0, "0100000000100000", "");
    fa.insertData("2012-09-06 18:00:00",       3.0,       3.0, "0100000000000000", "");
    fa.insertData("2012-09-07 06:00:00",       3.0,       3.0, "0100000000100000", "");
    fa.insertData("2012-09-07 12:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-07 18:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-08 06:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-08 12:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-08 18:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-09 06:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-10 06:00:00",  -32767.0,      12.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-10 12:00:00",       3.0,       3.0, "0100000000000000", "");
    fa.insertData("2012-09-12 06:00:00",       3.0,       3.0, "0100000000100000", "");
    fa.insertData("2012-09-12 12:00:00",       3.0,       3.0, "0100000000000000", "");
    fa.insertData("2012-09-13 06:00:00",      12.0,      12.0, "0100000000100000", "");
    fa.insertData("2012-09-14 06:00:00",       3.0,       3.0, "0100000000100000", "");
    fa.insertData("2012-09-15 06:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-15 12:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-15 18:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-16 06:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-16 12:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-16 18:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-17 06:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-17 12:00:00",       3.0,       3.0, "0100000000000000", "");
    fa.insertData("2012-09-17 18:00:00",       3.0,       3.0, "0100000000000000", "");
    fa.insertData("2012-09-18 06:00:00",       3.0,       3.0, "0100000000100000", "");
    fa.insertData("2012-09-20 18:00:00",  -32767.0,       3.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-21 06:00:00",  -32767.0,       3.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-21 18:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-22 06:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-23 06:00:00",  -32767.0,       3.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-23 12:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-23 18:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-24 06:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-25 06:00:00",      12.0,      12.0, "0100000000100000", "");
    fa.insertData("2012-09-27 18:00:00",  -32767.0,       3.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-28 18:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-29 18:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertParam = 35;
    fa.insertData("2012-08-31 06:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-09-03 12:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-09-03 18:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-09-04 06:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-09-04 12:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-09-04 18:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-09-05 06:00:00",       1.0,       1.0, "0100000000000000", "");
    fa.insertData("2012-09-05 12:00:00",       1.0,       1.0, "0100000000000000", "");
    fa.insertData("2012-09-05 18:00:00",       1.0,       1.0, "0100000000000000", "");
    fa.insertData("2012-09-06 06:00:00",       1.0,       1.0, "0100000000000000", "");
    fa.insertData("2012-09-06 18:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-09-07 06:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-09-10 12:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-09-12 06:00:00",       1.0,       1.0, "0100000000000000", "");
    fa.insertData("2012-09-12 12:00:00",       1.0,       1.0, "0100000000000000", "");
    fa.insertData("2012-09-13 06:00:00",       1.0,       1.0, "0100000000000000", "");
    fa.insertData("2012-09-14 06:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-09-17 12:00:00",       0.0,       0.0, "0100000000000000", "");
    fa.insertData("2012-09-17 18:00:00",       1.0,       1.0, "0100000000000000", "");
    fa.insertData("2012-09-18 06:00:00",       1.0,       1.0, "0100000000000000", "");
    fa.insertData("2012-09-23 12:00:00",  -32767.0,       0.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-23 18:00:00",  -32767.0,       0.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-25 06:00:00",       1.0,       1.0, "0100000000000000", "");
    fa.insertData("2012-09-27 18:00:00",  -32767.0,       0.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-29 18:00:00",  -32767.0,       0.0, "0000001000000005", "watchRR");
    fa.insertParam = 36;
    fa.insertData("2012-09-02 06:00:00",  -32767.0,       7.0, "0000001000000005", "watchRR");
    fa.insertParam = 37;
    fa.insertData("2012-09-02 06:00:00",  -32767.0,       0.0, "0000001000000005", "watchRR");
    fa.insertParam = 110;
    fa.insertData("2012-08-30 06:00:00",      -1.0,      -1.0, "0110000000001000", "");
    fa.insertData("2012-08-31 06:00:00",       1.0,       1.0, "0110000000001000", "");
    fa.insertData("2012-09-01 06:00:00",  -32767.0,       5.0, "0000001000007000", "QC1-7-110,QC2N_84190_83520_88100_83300_86950,QC2-redist,QC2N_84190_83520_88100_83300_86950,QC2-redist");
    fa.insertData("2012-09-02 06:00:00",  -32767.0,       0.2, "0000001000007000", "QC1-7-110,QC2N_84190_83520_88100_83300_86950,QC2-redist");
    fa.insertData("2012-09-03 06:00:00",      25.5,      20.3, "0110004000008000", "QC1-7-110,QC2N_88100_83300,QC2-redist,QC2-redist-endpoint,QC2N_84190_83520_88100_83300_86950,QC2-redist,QC2-redist-endpoint,QC2N_84190_83520_88100_83300_86950,QC2-redist,QC2-redist-endpoint");
    fa.insertData("2012-09-04 06:00:00",       4.2,       4.2, "0110000000001000", "");
    fa.insertData("2012-09-05 06:00:00",       7.9,       7.9, "0110000000001000", "");
    fa.insertData("2012-09-06 06:00:00",      29.4,      29.4, "0110000000001000", "");
    fa.insertData("2012-09-07 06:00:00",       1.6,       1.6, "0110000000001000", "");
    fa.insertData("2012-09-08 06:00:00",  -32767.0,       1.4, "0000001000007000", "QC1-7-110,QC2N_84190_83520_88100_83300_86950,QC2-redist");
    fa.insertData("2012-09-09 06:00:00",  -32767.0,       1.0, "0000001000007000", "QC1-7-110,QC2N_84190_83520_88100_83300_86950,QC2-redist");
    fa.insertData("2012-09-10 06:00:00",       2.5,       0.1, "0110004000008000", "QC1-7-110,QC2N_84190_83520_88100_83300_86950,QC2-redist,QC2-redist-endpoint,QC2N_84190_83520_88100_83300_86950,QC2-redist,QC2-redist-endpoint");
    fa.insertData("2012-09-11 06:00:00",       0.0,       0.0, "0110000000001000", "");
    fa.insertData("2012-09-12 06:00:00",       9.6,       9.6, "0110000000001000", "");
    fa.insertData("2012-09-13 06:00:00",       8.9,       8.9, "0110000000001000", "");
    fa.insertData("2012-09-14 06:00:00",       0.9,       0.9, "0110000000001000", "");
    fa.insertData("2012-09-15 06:00:00",  -32767.0,       3.2, "0000001000007000", "QC1-7-110,QC2N_84190_83520_88100_83300,QC2-redist,QC2N_84190_83520_88100_83300,QC2-redist,QC2N_84190_83520_88100_83300,QC2-redist");
    fa.insertData("2012-09-16 06:00:00",  -32767.0,       5.9, "0000001000007000", "QC1-7-110,QC2N_84190_83520_88100_83300,QC2-redist,QC2N_84190_83520_88100_83300,QC2-redist");
    fa.insertData("2012-09-17 06:00:00",      20.0,      10.9, "0110004000008000", "QC1-7-110,QC2N_84190_83300,QC2-redist,QC2-redist-endpoint,QC2N_84190_83520_88100_83300,QC2-redist,QC2-redist-endpoint,QC2N_84190_83520_88100_83300_86950,QC2-redist,QC2-redist-endpoint,QC2N_84190_83520_88100_83300_86950,QC2-redist,QC2-redist-endpoint");
    fa.insertData("2012-09-18 06:00:00",      14.7,      14.7, "0110000000001000", "");
    fa.insertData("2012-09-19 06:00:00",      -1.0,      -1.0, "0110000000001000", "");
    fa.insertData("2012-09-20 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-21 06:00:00",      -1.0,      17.0, "000000400000A006", "watchRR,watchRR");
    fa.insertData("2012-09-22 06:00:00",  -32767.0,       0.9, "0000001000007006", "QC1-7-110,QC2N_84190_83520_88100_83300,QC2-redist,QC2N_84190_83520_88100_83300_86950,QC2-redist,watchRR");
    fa.insertData("2012-09-23 06:00:00",      18.9,       1.0, "000000400000A006", "watchRR,watchRR");
    fa.insertData("2012-09-24 06:00:00",      13.0,      11.1, "0110004000007006", "QC1-7-110,QC2N_84190_83520_88100_83300,QC2-redist,QC2-redist-endpoint,watchRR,watchRR");
    fa.insertData("2012-09-25 06:00:00",       0.2,       0.2, "0110000000001000", "");
    fa.insertData("2012-09-26 06:00:00",  -32767.0,      -1.0, "0000001000007000", "QC1-7-110,QC2N_83880_84190_87860_83520_88100,QC2-redist");
    fa.insertData("2012-09-27 06:00:00",  -32767.0,      -1.0, "0000001000007000", "QC1-7-110,QC2N_83880_84190_87860_83520_88100,QC2-redist");
    fa.insertData("2012-09-28 06:00:00",  -32767.0,       0.0, "0000001000007000", "QC1-7-110,QC2N_84190_87860_83520_88100_83300,QC2-redist");
    fa.insertData("2012-09-29 06:00:00",  -32767.0,       3.2, "0000001000007000", "QC1-7-110,QC2N_84190_87860_83520_83300_86950,QC2-redist");
    fa.insertData("2012-09-30 06:00:00",  -32767.0,       0.3, "0000001000007000", "QC1-7-110,QC2N_84190_83520_83300_86950,QC2-redist");
    fa.insertParam = 112;
    fa.insertData("2012-08-30 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-08-31 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-01 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-02 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-03 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-04 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-05 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-06 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-07 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-08 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-09 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-10 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-11 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-12 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-13 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-14 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-15 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-16 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-17 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-18 06:00:00",      -1.0,      -1.0, "0110000000300000", "QC1-6-66.b_66.c");
    fa.insertData("2012-09-19 06:00:00",       2.0,       2.0, "0140000000300000", "QC1-2-72.a,QC1-2-72.b6,QC1-6-66.b_66.c");
    fa.insertData("2012-09-20 06:00:00",      -1.0,      -1.0, "0110000000000000", "");
    fa.insertData("2012-09-21 06:00:00",      -1.0,      -1.0, "0000000000000001", "watchRR");
    fa.insertData("2012-09-22 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-23 06:00:00",      -1.0,      -1.0, "0000000000000001", "watchRR");
    fa.insertData("2012-09-24 06:00:00",      -1.0,      -1.0, "0110000000100001", "watchRR");
    fa.insertData("2012-09-25 06:00:00",      -1.0,      -1.0, "0110000000100000", "");
    fa.insertData("2012-09-26 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-27 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-28 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-29 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");
    fa.insertData("2012-09-30 06:00:00",  -32767.0,      -1.0, "0000001000000005", "watchRR");

    fa.insertStation = 84070;
    fa.insertParam = 110;
    fa.insertModel("2012-08-30 06:00:00",       0.1);
    fa.insertModel("2012-08-30 18:00:00",       0.0);
    fa.insertModel("2012-08-31 06:00:00",       0.2);
    fa.insertModel("2012-08-31 18:00:00",      24.7);
    fa.insertModel("2012-09-01 06:00:00",      13.1);
    fa.insertModel("2012-09-01 18:00:00",       0.8);
    fa.insertModel("2012-09-02 06:00:00",       0.0);
    fa.insertModel("2012-09-02 18:00:00",       0.0);
    fa.insertModel("2012-09-03 06:00:00",      21.9);
    fa.insertModel("2012-09-03 18:00:00",      12.6);
    fa.insertModel("2012-09-04 06:00:00",       8.7);
    fa.insertModel("2012-09-04 18:00:00",      16.0);
    fa.insertModel("2012-09-05 06:00:00",       8.8);
    fa.insertModel("2012-09-05 18:00:00",       2.6);
    fa.insertModel("2012-09-06 06:00:00",      16.7);
    fa.insertModel("2012-09-06 18:00:00",      43.8);
    fa.insertModel("2012-09-07 06:00:00",       6.2);
    fa.insertModel("2012-09-07 18:00:00",       9.1);
    fa.insertModel("2012-09-08 06:00:00",       0.0);
    fa.insertModel("2012-09-09 06:00:00",       4.0);
    fa.insertModel("2012-09-09 18:00:00",       1.0);
    fa.insertModel("2012-09-10 06:00:00",       0.0);
    fa.insertModel("2012-09-10 18:00:00",       0.3);
    fa.insertModel("2012-09-11 06:00:00",       0.1);
    fa.insertModel("2012-09-11 18:00:00",       0.0);
    fa.insertModel("2012-09-12 06:00:00",       0.4);
    fa.insertModel("2012-09-12 18:00:00",      31.4);
    fa.insertModel("2012-09-13 06:00:00",      16.5);
    fa.insertModel("2012-09-13 18:00:00",       0.4);
    fa.insertModel("2012-09-14 06:00:00",       6.2);
    fa.insertModel("2012-09-14 18:00:00",       5.4);
    fa.insertModel("2012-09-15 06:00:00",       0.8);
    fa.insertModel("2012-09-15 18:00:00",       3.9);
    fa.insertModel("2012-09-16 06:00:00",       2.0);
    fa.insertModel("2012-09-16 18:00:00",      19.9);
    fa.insertModel("2012-09-17 06:00:00",      23.8);
    fa.insertModel("2012-09-17 18:00:00",       5.1);
    fa.insertModel("2012-09-18 06:00:00",       6.8);
    fa.insertModel("2012-09-18 18:00:00",       2.0);
    fa.insertModel("2012-09-19 06:00:00",       0.2);
    fa.insertModel("2012-09-19 18:00:00",       0.0);
    fa.insertModel("2012-09-20 06:00:00",       0.0);
    fa.insertModel("2012-09-20 18:00:00",       0.2);
    fa.insertModel("2012-09-21 06:00:00",       6.7);
    fa.insertModel("2012-09-21 18:00:00",       8.7);
    fa.insertModel("2012-09-22 06:00:00",       0.7);
    fa.insertModel("2012-09-22 18:00:00",       3.3);
    fa.insertModel("2012-09-23 06:00:00",       5.7);
    fa.insertModel("2012-09-23 18:00:00",      10.9);
    fa.insertModel("2012-09-24 06:00:00",      18.2);
    fa.insertModel("2012-09-24 18:00:00",       7.4);
    fa.insertModel("2012-09-25 06:00:00",       0.5);
    fa.insertModel("2012-09-25 18:00:00",       0.0);
    fa.insertModel("2012-09-26 06:00:00",       0.0);
    fa.insertModel("2012-09-26 18:00:00",       0.0);
    fa.insertModel("2012-09-27 06:00:00",       0.0);
    fa.insertModel("2012-09-27 18:00:00",       0.0);
    fa.insertModel("2012-09-28 06:00:00",       0.3);
    fa.insertModel("2012-09-28 18:00:00",       0.2);
    fa.insertModel("2012-09-29 06:00:00",       0.8);
    fa.insertModel("2012-09-29 18:00:00",       1.1);
    fa.insertModel("2012-09-30 06:00:00",       0.0);

    fa.mKvStations.push_back(kvalobs::kvStation(84070, 68.330200, 16.788300, 53.000000, 0.0f, "BJØRKÅSEN", 0, 0, "?", "?", "?", 10, true, timeutil::to_miTime(timeutil::from_iso_extended_string("1964-01-01 00:00:00"))));

    fa.mObsPgm.push_back(kvalobs::kvObsPgm(84070, 110, 0, 1, 302, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1,
                                           timeutil::to_miTime(timeutil::from_iso_extended_string("2005-01-01 00:00:00")),
                                           timeutil::to_miTime(timeutil::ptime())));

    {
        const TimeRange t = t_84070_20120930();
        fa.kda->addSubscription(ObsSubscription(84070, t));
    }
}
#endif // !LOAD_DECL_ONLY
