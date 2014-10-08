#include "common/test/FakeKvApp.hh"
#include "common/test/TestHelpers.hh"
#include "common/KvalobsAccess.hh"
inline TimeSpan t_3200_20141008()
    { return TimeSpan(s2t("2014-10-06 06:00:00"), s2t("2014-10-08 12:00:00")); }
#ifdef LOAD_DECL_ONLY
void load_3200_20141008(FakeKvApp& fa);
#else // LOAD_DECL_ONLY
void load_3200_20141008(FakeKvApp& fa)
{
    fa.insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_3200_20141008.txt");

    fa.insertModelFromFile(std::string(TEST_SOURCE_DIR)+"/model_3200_20141008.txt");

    fa.addStation("5590;12.0067;60.1903;148.0;KONGSVINGER;0;2006-07-01 00:00:00");
    fa.addStation("4460;10.829;60.1173;170.0;HAKADAL JERNBANESTASJON;0;2007-01-08 00:00:00");
    fa.addStation("2650;11.58;59.9122;128.0;AURSKOG II;0;2007-11-28 00:00:00");
    fa.addStation("18020;10.7857;59.8783;135.0;OSLO - LAMBERTSETER;0;1985-05-15 00:00:00");
    fa.addStation("3200;11.1338;59.3072;31.0;BATER�D;0;1984-06-15 00:00:00");
    fa.addStation("7420;11.4992;61.3763;872.0;RENA - �RNHAUGEN;0;2014-02-07 00:00:00");

    fa.addObsPgm("5590;61;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2006-07-01 00:00:00;null");
    fa.addObsPgm("4460;61;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-01-08 00:00:00;2007-11-01 00:00:00");
    fa.addObsPgm("4460;61;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-01 00:00:00;null");
    fa.addObsPgm("2650;61;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-28 00:00:00;null");
    fa.addObsPgm("4460;61;0;1;506;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2013-01-21 00:00:00;null");
    fa.addObsPgm("7420;61;0;1;501;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2014-01-28 00:00:00;null");
    fa.addObsPgm("7420;61;0;1;506;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2014-07-29 00:00:00;null");
    fa.addObsPgm("5590;81;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2006-07-01 00:00:00;null");
    fa.addObsPgm("4460;81;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-01-08 00:00:00;2007-11-01 00:00:00");
    fa.addObsPgm("4460;81;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-01 00:00:00;null");
    fa.addObsPgm("2650;81;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-28 00:00:00;null");
    fa.addObsPgm("4460;81;0;1;506;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2013-01-21 00:00:00;null");
    fa.addObsPgm("7420;81;0;1;501;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2014-01-28 00:00:00;null");
    fa.addObsPgm("7420;81;0;1;506;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2014-07-29 00:00:00;null");
    fa.addObsPgm("18020;105;0;1;4;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1985-05-15 00:00:00;2013-06-18 00:00:00");
    fa.addObsPgm("18020;105;0;1;504;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2011-06-29 00:00:00;null");
    fa.addObsPgm("4460;108;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-01 00:00:00;2014-06-07 00:00:00");
    fa.addObsPgm("4460;108;0;1;342;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2014-06-07 00:00:00;null");
    fa.addObsPgm("4460;109;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-01 00:00:00;2014-06-07 00:00:00");
    fa.addObsPgm("4460;109;0;1;342;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2014-06-07 00:00:00;null");
    fa.addObsPgm("3200;110;0;1;402;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;1957-01-01 06:00:00;2011-10-24 00:00:00");
    fa.addObsPgm("4460;110;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-01 00:00:00;2014-06-07 00:00:00");
    fa.addObsPgm("3200;110;0;1;302;0;0;0;0;0;0;0;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2011-10-24 00:00:00;null");
    fa.addObsPgm("4460;110;0;1;342;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;1;1;1;1;1;1;1;2014-06-07 00:00:00;null");
    fa.addObsPgm("18020;211;0;1;4;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2004-12-24 00:00:00;2009-01-01 00:00:00");
    fa.addObsPgm("5590;211;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2006-07-01 00:00:00;null");
    fa.addObsPgm("4460;211;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-01-08 00:00:00;2007-11-01 00:00:00");
    fa.addObsPgm("4460;211;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-01 00:00:00;null");
    fa.addObsPgm("2650;211;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-28 00:00:00;null");
    fa.addObsPgm("18020;211;0;1;504;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-12-07 00:00:00;2011-08-08 00:00:00");
    fa.addObsPgm("18020;211;0;1;514;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-08-08 00:00:00;null");
    fa.addObsPgm("7420;211;0;1;501;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2014-01-28 00:00:00;null");
    fa.addObsPgm("7420;211;0;1;506;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2014-07-29 00:00:00;null");
    fa.addObsPgm("5590;213;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2006-07-01 00:00:00;null");
    fa.addObsPgm("4460;213;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-01-08 00:00:00;2007-11-01 00:00:00");
    fa.addObsPgm("4460;213;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-01 00:00:00;null");
    fa.addObsPgm("2650;213;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-28 00:00:00;null");
    fa.addObsPgm("18020;213;0;1;504;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-12-07 00:00:00;2011-08-08 00:00:00");
    fa.addObsPgm("18020;213;0;1;514;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-08-08 00:00:00;null");
    fa.addObsPgm("7420;213;0;1;501;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2014-02-18 00:00:00;null");
    fa.addObsPgm("5590;215;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2006-07-01 00:00:00;null");
    fa.addObsPgm("4460;215;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-01-08 00:00:00;2007-11-01 00:00:00");
    fa.addObsPgm("4460;215;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-01 00:00:00;null");
    fa.addObsPgm("2650;215;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-28 00:00:00;null");
    fa.addObsPgm("18020;215;0;1;504;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2010-12-07 00:00:00;2011-08-08 00:00:00");
    fa.addObsPgm("18020;215;0;1;514;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2011-08-08 00:00:00;null");
    fa.addObsPgm("7420;215;0;1;501;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2014-01-28 00:00:00;null");
    fa.addObsPgm("5590;262;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2006-07-01 00:00:00;null");
    fa.addObsPgm("4460;262;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-01-08 00:00:00;2007-11-01 00:00:00");
    fa.addObsPgm("4460;262;0;1;342;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-01 00:00:00;null");
    fa.addObsPgm("2650;262;0;1;330;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2007-11-28 00:00:00;null");
    fa.addObsPgm("7420;262;0;1;501;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;2014-01-28 00:00:00;null");

    fa.addTypes(kvalobs::kvTypes(330,"SMS meldingsformat 30",10,20,"A","h","Automatstasjoner"));
    fa.addTypes(kvalobs::kvTypes(342,"SMS meldingsformat 42",10,20,"A","h","Automatstasjoner med litt flere parametere"));
    fa.addTypes(kvalobs::kvTypes(501,"met.no AVS, generelt format",10,10,"A","h",""));
    fa.addTypes(kvalobs::kvTypes(514,"Ekstern holder AVS, dagsoppfoelging 2. melding",10,1700,"A","m",""));
    fa.addTypes(kvalobs::kvTypes(302,"SMS meldingsformat 2",30,1080,"M","h","Nedboermelding m param SD"));
    fa.addTypes(kvalobs::kvTypes(402,"ukekort",0,38900,"M","h","etterregistrert"));
    fa.addTypes(kvalobs::kvTypes(4,"minuttsvise automatdata",0,420,"A","m","pluviometer"));
    fa.addTypes(kvalobs::kvTypes(504,"AVS, minuttsmeldinger",10,1700,"A","m",""));
    fa.addTypes(kvalobs::kvTypes(506,"met.no AVS, timinuttsmeldinger",10,80,"A","10m",""));

    fa.addParam(kvalobs::kvParam(105,"RR_01","Nedb�r, tilvekst siste minutt","mm",0,"1 min akkumulert"));
    fa.addParam(kvalobs::kvParam(61,"DD","Vindretning 10 minutt","vektoriell i grader",0,"Tilh�rer FF. SYNOP-kodeverdi 99 blir lagret i Kvalobs som -3"));
    fa.addParam(kvalobs::kvParam(81,"FF","Vindhastighet 10 minutt","m/s",0,"10 minutt n�verdi"));
    fa.addParam(kvalobs::kvParam(213,"TAN","Temperatur minimum i timen","grad C",0,"minimum minuttverdi i timen"));
    fa.addParam(kvalobs::kvParam(215,"TAX","Temperatur maksimum i timen","grad C",0,"maksimum minuttverdi i timen"));
    fa.addParam(kvalobs::kvParam(109,"RR_12","Nedb�r, tilvekst siste 12 timer","mm",0,"Verdien -1 angir at nedb�r ikke er observert"));
    fa.addParam(kvalobs::kvParam(110,"RR_24","Nedb�r, tilvekst siste 24 timer","mm",0,"Verdien -1 angir at nedb�r ikke er observert"));
    fa.addParam(kvalobs::kvParam(108,"RR_6","Nedb�r, tilvekst siste 6 timer","mm",0,"Verdien -1 angir at nedb�r ikke er observert"));
    fa.addParam(kvalobs::kvParam(211,"TA","Temperatur","grad C",0,"1 minutt middelverdi"));
    fa.addParam(kvalobs::kvParam(262,"UU","Relativ luftfuktighet","prosent",0,"N�verdi"));

}
#endif // !LOAD_DECL_ONLY

// generated by '/home/alexanderb/tools/kvhqc/fetch_test_data.py' '--params' '81,61,211,213,215,262,110' '3200,2650,4460,5590,7420,18020' '2014-10-06 06:00:00' '2014-10-08 12:00:00' on 2014-10-08T17:12:44.665351
// Local Variables:
// buffer-read-only: t
// End:
