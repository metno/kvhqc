#include "common/test/FakeKvApp.hh"
#include "common/KvalobsAccess.hh"
inline TimeRange t_examples_201303()
    { return TimeRange(timeutil::from_iso_extended_string("2013-02-01 00:00:00"), timeutil::from_iso_extended_string("2013-04-04 12:00:00")); }
#ifdef LOAD_DECL_ONLY
void load_examples_201303(FakeKvApp& fa);
#else // LOAD_DECL_ONLY
void load_examples_201303(FakeKvApp& fa)
{
    fa.insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_examples_201303.txt");

    fa.insertModelFromFile(std::string(TEST_SOURCE_DIR)+"/model_examples_201303.txt");

    fa.addStation("76930;8.085;66.0256;33.0;NORNE;5;1998-03-01 00:00:00");
    fa.addStation("23550;8.9228;61.2508;965.0;BEITOST�LEN II;0;2010-08-28 00:00:00");
    fa.addStation("15730;7.8953;61.8958;664.0;BR�T� - SLETTOM;8;2010-07-01 00:00:00");
    fa.addStation("1650;11.6599;59.3006;113.0;STR�MSFOSS SLUSE;9;1883-01-01 00:00:00");
    fa.addStation("18700;10.72005;59.9423;94.0;OSLO - BLINDERN;8;1937-02-25 00:00:00");
    fa.addStation("98360;29.7;70.6028;152.0;B�TSFJORD - STRAUMSNESAKSLA;8;1999-09-09 00:00:00");
    fa.addStation("99720;25.0133;76.5097;6.0;HOPEN;8;1944-11-01 00:00:00");
    fa.addStation("59610;5.5817;62.103;41.0;FISK�BYGD;8;1969-07-01 00:00:00");
    fa.addStation("13640;9.4085;61.5162;630.0;OLSTAPPEN;10;1970-10-01 00:00:00");
    fa.addStation("13700;9.5347;61.4167;752.0;ESPEDALEN;9;1942-10-01 00:00:00");
    fa.addStation("13150;10.1857;61.455;200.0;F�VANG;8;2009-11-18 00:00:00");
    fa.addStation("17980;10.8236;59.8427;92.0;OSLO - LJABRUVEIEN;9;2000-01-01 00:00:00");
    fa.addStation("90510;19.0065;69.6508;3.0;TROMSDALEN;0;2011-10-20 00:00:00");
    fa.addStation("14200;9.1541;61.7247;599.0;LEIRFLATEN;0;2012-08-25 00:00:00");
    fa.addStation("7950;11.3705;61.1858;255.0;RENA FLYPLASS;8;2012-06-15 00:00:00");
    fa.addStation("18210;10.804;59.923;100.0;OSLO - HOVIN;9;2011-11-22 00:00:00");
    fa.addStation("91130;20.094;69.5588;710.0;LYNGEN - GJERDVASSBU;0;2011-11-17 00:00:00");
    fa.addStation("52640;5.5937;60.874;7.0;MATRE KRAFTSTASJON;10;1975-07-01 00:00:00");

    fa.addObsPgm("17980;215;0;1;514;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-08-09 00:00:00;null");
    fa.addObsPgm("13700;38;0;1;302;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2004-12-24 00:00:00;null");
    fa.addObsPgm("91130;211;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-11-17 00:00:00;null");
    fa.addObsPgm("91130;213;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-11-17 00:00:00;null");
    fa.addObsPgm("91130;215;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-11-17 00:00:00;null");
    fa.addObsPgm("18700;178;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2006-04-03 00:00:00;null");
    fa.addObsPgm("18700;213;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2006-04-03 00:00:00;null");
    fa.addObsPgm("18700;215;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2006-04-03 00:00:00;null");
    fa.addObsPgm("18700;262;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2006-04-03 00:00:00;null");
    fa.addObsPgm("91130;112;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-11-17 00:00:00;null");
    fa.addObsPgm("76930;262;0;1;11;0;1;0;0;0;0;0;1;0;0;0;0;0;1;0;0;0;0;0;1;0;0;0;0;0;1;1;1;1;1;1;1;2010-01-04 00:00:00;null");
    fa.addObsPgm("7950;178;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-12-09 00:00:00;null");
    fa.addObsPgm("7950;211;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-12-09 00:00:00;null");
    fa.addObsPgm("7950;213;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-12-09 00:00:00;null");
    fa.addObsPgm("7950;215;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-12-09 00:00:00;null");
    fa.addObsPgm("76930;178;0;1;22;0;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;1;1;1;1;1;1;1;2006-10-01 00:00:00;null");
    fa.addObsPgm("76930;211;0;1;22;0;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;1;1;1;1;1;1;1;2006-10-01 00:00:00;null");
    fa.addObsPgm("76930;213;0;1;22;0;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;1;1;1;1;1;1;1;2006-10-01 00:00:00;null");
    fa.addObsPgm("76930;211;0;1;11;0;1;0;0;1;0;0;1;0;0;1;0;0;1;0;0;1;0;0;1;0;0;1;0;0;1;1;1;1;1;1;1;2004-12-24 00:00:00;null");
    fa.addObsPgm("76930;215;0;1;22;0;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;1;1;1;1;1;1;1;2006-10-01 00:00:00;null");
    fa.addObsPgm("76930;262;0;1;22;0;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;0;1;1;1;1;1;1;1;1;1;2006-10-01 00:00:00;null");
    fa.addObsPgm("59610;109;0;1;312;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;1;0;0;0;0;0;1;1;1;1;1;1;1;2007-06-28 00:00:00;null");
    fa.addObsPgm("7950;262;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-12-09 00:00:00;null");
    fa.addObsPgm("7950;106;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-12-09 00:00:00;null");
    fa.addObsPgm("18700;109;0;1;308;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;1;0;0;0;0;0;1;1;1;1;1;1;1;2006-04-03 00:00:00;null");
    fa.addObsPgm("18700;112;0;1;308;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2006-04-03 00:00:00;null");
    fa.addObsPgm("90510;105;0;1;4;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2012-05-04 00:00:00;null");
    fa.addObsPgm("17980;211;0;1;514;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-08-09 00:00:00;null");
    fa.addObsPgm("17980;213;0;1;514;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-08-09 00:00:00;null");
    fa.addObsPgm("18210;213;0;1;514;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-08-08 00:00:00;null");
    fa.addObsPgm("18210;215;0;1;514;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-08-08 00:00:00;null");
    fa.addObsPgm("99720;178;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-08-18 00:00:00;null");
    fa.addObsPgm("99720;211;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-08-18 00:00:00;null");
    fa.addObsPgm("99720;213;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-08-18 00:00:00;null");
    fa.addObsPgm("99720;215;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-08-18 00:00:00;null");
    fa.addObsPgm("99720;262;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-08-18 00:00:00;null");
    fa.addObsPgm("59610;36;0;1;312;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2004-12-24 00:00:00;null");
    fa.addObsPgm("59610;38;0;1;312;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2004-12-24 00:00:00;null");
    fa.addObsPgm("18210;211;0;1;502;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-11-22 00:00:00;null");
    fa.addObsPgm("76930;178;0;1;11;0;1;0;0;1;0;0;1;0;0;1;0;0;1;0;0;1;0;0;1;0;0;1;0;0;1;1;1;1;1;1;1;2004-12-24 00:00:00;null");
    fa.addObsPgm("18210;211;25;1;502;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-11-22 00:00:00;null");
    fa.addObsPgm("1650;34;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2004-12-24 00:00:00;null");
    fa.addObsPgm("1650;36;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2004-12-24 00:00:00;null");
    fa.addObsPgm("1650;38;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2004-12-24 00:00:00;null");
    fa.addObsPgm("18210;262;0;1;502;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-11-22 00:00:00;null");
    fa.addObsPgm("99720;107;0;1;308;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2010-08-18 00:00:00;null");
    fa.addObsPgm("99720;109;0;1;308;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;1;0;0;0;0;0;1;1;1;1;1;1;1;2010-08-18 00:00:00;null");
    fa.addObsPgm("99720;110;0;1;308;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2010-08-18 00:00:00;null");
    fa.addObsPgm("13150;211;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2009-11-18 00:00:00;null");
    fa.addObsPgm("13150;213;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2009-11-18 00:00:00;null");
    fa.addObsPgm("13150;215;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2009-11-18 00:00:00;null");
    fa.addObsPgm("13640;36;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2004-12-24 00:00:00;null");
    fa.addObsPgm("13640;38;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2004-12-24 00:00:00;null");
    fa.addObsPgm("13150;262;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2009-11-18 00:00:00;null");
    fa.addObsPgm("99720;108;0;1;308;0;1;0;0;0;0;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2010-08-18 00:00:00;null");
    fa.addObsPgm("17980;105;0;1;4;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2000-07-01 00:00:00;null");
    fa.addObsPgm("18700;211;0;2;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-12-12 00:00:00;null");
    fa.addObsPgm("23550;112;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-08-28 00:00:00;null");
    fa.addObsPgm("98360;178;0;1;311;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2000-04-06 00:00:00;null");
    fa.addObsPgm("98360;211;0;1;311;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2000-04-06 00:00:00;null");
    fa.addObsPgm("98360;213;0;1;311;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2000-04-06 00:00:00;null");
    fa.addObsPgm("98360;215;0;1;311;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2000-04-06 00:00:00;null");
    fa.addObsPgm("90510;106;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-10-20 00:00:00;null");
    fa.addObsPgm("98360;262;0;1;311;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2000-04-06 00:00:00;null");
    fa.addObsPgm("90510;211;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-10-20 00:00:00;null");
    fa.addObsPgm("90510;213;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-10-20 00:00:00;null");
    fa.addObsPgm("90510;215;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-10-20 00:00:00;null");
    fa.addObsPgm("23550;211;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-08-28 00:00:00;null");
    fa.addObsPgm("23550;213;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-08-28 00:00:00;null");
    fa.addObsPgm("23550;215;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-08-28 00:00:00;null");
    fa.addObsPgm("23550;262;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-08-28 00:00:00;null");
    fa.addObsPgm("23550;106;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-08-28 00:00:00;null");
    fa.addObsPgm("17980;105;0;1;504;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2011-06-29 00:00:00;null");
    fa.addObsPgm("18700;106;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2006-04-03 00:00:00;null");
    fa.addObsPgm("15730;211;0;1;312;0;0;0;0;1;0;0;1;0;0;0;0;0;1;0;0;0;0;0;1;0;0;1;0;0;1;1;1;1;1;1;1;2012-06-01 00:00:00;null");
    fa.addObsPgm("15730;262;0;1;312;0;0;0;0;1;0;0;1;0;0;0;0;0;1;0;0;0;0;0;1;0;0;1;0;0;1;1;1;1;1;1;1;2012-06-01 00:00:00;null");
    fa.addObsPgm("14200;106;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2012-08-25 00:00:00;null");
    fa.addObsPgm("14200;112;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2012-08-25 00:00:00;null");
    fa.addObsPgm("14200;211;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2012-08-25 00:00:00;null");
    fa.addObsPgm("14200;213;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2012-08-25 00:00:00;null");
    fa.addObsPgm("14200;215;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2012-08-25 00:00:00;null");
    fa.addObsPgm("14200;262;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2012-08-25 00:00:00;null");
    fa.addObsPgm("18210;105;0;1;4;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1999-01-15 00:00:00;null");
    fa.addObsPgm("18210;105;0;1;504;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2011-06-29 00:00:00;null");
    fa.addObsPgm("15730;112;0;1;312;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1998-10-31 06:00:00;null");
    fa.addObsPgm("15730;109;0;1;312;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;1;0;0;0;0;0;1;1;1;1;1;1;1;1998-10-31 06:00:00;null");
    fa.addObsPgm("59610;112;0;1;312;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1969-07-01 06:00:00;null");
    fa.addObsPgm("59610;262;0;1;312;0;0;0;0;0;0;0;1;0;0;0;0;0;1;0;0;0;0;0;1;0;0;0;0;0;1;1;1;1;1;1;1;1969-07-01 06:00:00;null");
    fa.addObsPgm("59610;211;0;1;312;0;0;0;0;0;0;0;1;0;0;0;0;0;1;0;0;0;0;0;1;0;0;0;0;0;1;1;1;1;1;1;1;1969-07-01 06:00:00;null");
    fa.addObsPgm("59610;34;0;1;312;0;0;0;0;0;0;0;1;0;0;0;0;0;1;0;0;0;0;0;1;0;0;0;0;0;1;1;1;1;1;1;1;1969-07-01 06:00:00;null");
    fa.addObsPgm("13700;112;0;1;302;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2002-09-01 00:00:00;null");
    fa.addObsPgm("13700;35;0;1;302;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2002-09-01 00:00:00;null");
    fa.addObsPgm("13700;34;0;1;302;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2002-09-01 00:00:00;null");
    fa.addObsPgm("13700;18;0;1;302;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2002-09-01 00:00:00;null");
    fa.addObsPgm("13700;39;0;1;302;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2002-09-01 00:00:00;null");
    fa.addObsPgm("13700;36;0;1;302;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2002-09-01 00:00:00;null");
    fa.addObsPgm("13700;37;0;1;302;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2002-09-01 00:00:00;null");
    fa.addObsPgm("13700;110;0;1;302;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2002-09-01 00:00:00;null");
    fa.addObsPgm("13640;112;0;1;402;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1970-10-01 06:00:00;null");
    fa.addObsPgm("13640;35;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1970-09-30 12:00:00;null");
    fa.addObsPgm("13640;34;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1993-10-09 06:00:00;null");
    fa.addObsPgm("13640;18;0;1;402;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1970-10-01 06:00:00;null");
    fa.addObsPgm("13640;39;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1970-09-30 12:00:00;null");
    fa.addObsPgm("13640;37;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1970-09-30 12:00:00;null");
    fa.addObsPgm("13640;110;0;1;402;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1970-10-01 06:00:00;null");
    fa.addObsPgm("1650;112;0;1;402;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1896-01-01 06:00:00;null");
    fa.addObsPgm("1650;35;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1883-01-01 12:00:00;null");
    fa.addObsPgm("1650;18;0;1;402;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1896-01-01 06:00:00;null");
    fa.addObsPgm("1650;39;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1883-01-01 12:00:00;null");
    fa.addObsPgm("1650;37;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1883-01-01 12:00:00;null");
    fa.addObsPgm("1650;110;0;1;402;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1883-01-01 06:00:00;null");
}
#endif // !LOAD_DECL_ONLY

// generated by '/home/alexanderb/tools/kvhqc/fetch_test_data.py' '-d' '/tmp/ex' '--name' 'examples_201303' '18700,18210,17980,13640,13700,52640,1650,76930,98360,99720,90510,91130,7950,59610,13150,14200,23550,15730' '2013-02-01T00:00' '2013-04-04T12:00' on 2013-04-04T12:49:04.083103
// Local Variables:
// buffer-read-only: t
// End:
