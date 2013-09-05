
#include "StInfoSysBuffer.hh"

#include "Functors.hh"
#include "KvMetaDataBuffer.hh"

#include <miconfparser/confexception.h>
#include <miconfparser/confsection.h>
#include <miconfparser/valelement.h>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtCore/QCoreApplication>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtGui/QMessageBox>

#include <boost/foreach.hpp>
#include <boost/range.hpp>

#include <iomanip>
#include <fstream>
#include <set>

#define MILOGGER_CATEGORY "kvhqc.StInfoSysBuffer"
#include "HqcLogging.hh"

namespace /*anonymous*/ {

const char QSQLNAME_REMOTE[] = "stinfosys";

struct countyInfo {
    int stnr;
    QString name;
    QString county;
    QString municip;
    QString web;
    QString pri;
    QString ki;
};

using namespace miutil::conf;

bool connect2stinfosys(miutil::conf::ConfSection *conf)
{
    METLIBS_LOG_SCOPE();
    const ValElementList valHost     = conf->getValue("stinfosys.host");
    const ValElementList valDbname   = conf->getValue("stinfosys.dbname");
    const ValElementList valUser     = conf->getValue("stinfosys.user");
    const ValElementList valPassword = conf->getValue("stinfosys.password");
    const ValElementList valPort     = conf->getValue("stinfosys.port");
    
    if (valHost.size() != 1 or valDbname.size() != 1 or valUser.size() != 1 or valPassword.size() != 1 or valPort.size() != 1)
        return false;
    
    try {
        QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", QSQLNAME_REMOTE);
        db.setHostName    (QString::fromStdString(valHost    .front().valAsString()));
        db.setDatabaseName(QString::fromStdString(valDbname  .front().valAsString()));
        db.setUserName    (QString::fromStdString(valUser    .front().valAsString()));
        db.setPassword    (QString::fromStdString(valPassword.front().valAsString()));
        db.setPort        (valPort.front().valAsInt());
        return db.open();
    } catch (miutil::conf::InvalidTypeEx& e) {
        return false;
    }
}
} // anonymous namespace

/*!
  Read station info from the stinfosys database
*/
bool StInfoSysBuffer::readFromStInfoSys()
{
    METLIBS_LOG_SCOPE();
    if (not isConnected())
        return false;

    const int webs_ids[] = {
        4780, 7010, 10380, 16610, 17150, 18700, 23420, 24890, 27500,
        31620, 36200, 39100, 42160, 44560, 47300, 50500, 50540, 54120,
        59800, 60500, 69100, 71550, 75410, 75550, 80610, 82290, 85380,
        87110, 89350, 90450, 93140, 93700, 94500, 96400, 98550, 99370,
        99710, 99720, 99840, 99910, 99950 };
    const std::set<int> webs(webs_ids, boost::end(webs_ids));

    const int pri1s_ids[] = {
        4780, 10380, 18500, 18700, 24890, 27500, 36200, 39040, 44560,
        50540, 54120, 60500, 69100, 70850, 72800, 82290, 90450, 93700,
        97251, 98550, 99710, 99720, 99840, 99910, 99950 };
    const std::set<int> pri1s(pri1s_ids, boost::end(pri1s_ids));

    const int pri2s_ids[] = {
        3190, 17150, 28380, 35860, 60990, 63420, 68860, 68863, 93140, 94500, 95350 };
    const std::set<int> pri2s(pri2s_ids, boost::end(pri2s_ids));
    
    const int pri3s_ids[] = {
        180, 700, 1130, 2540, 4440, 4460, 6020, 7010, 8140, 9580, 11500, 12320,
        12550, 12680, 13160, 13420, 13670, 15730, 16610, 16740, 17000, 18950,
        19710, 20301, 21680, 23160, 23420, 23500, 23550, 25110, 25830, 26900,
        26990, 27450, 27470, 28800, 29720, 30420, 30650, 31620, 32060, 33890,
        34130, 36560, 37230, 38140, 39100, 39690, 40880, 41110, 41670, 41770,
        42160, 42920, 43010, 44080, 44610, 45870, 46510, 46610, 46910, 47260,
        47300, 48120, 48330, 49800, 50070, 50300, 50500, 51530, 51800, 52290,
        52535, 52860, 53101, 55290, 55700, 55820, 57420, 57710, 57770, 58070,
        58900, 59110, 59610, 59680, 59800, 61180, 61770, 62270, 62480, 63705,
        64330, 64550, 65310, 65940, 66730, 68340, 69150, 69380, 70150, 71000,
        71850, 71990, 72060, 72580, 73500, 75220, 75410, 75550, 76330, 76450,
        76530, 76750, 77230, 77550, 78800, 79600, 80101, 80102, 80610, 80700,
        81680, 82410, 83550, 85380, 85450, 85891, 85892, 86500, 86740, 87110,
        87640, 88200, 88690, 89350, 90490, 90800, 91380, 91760, 92350, 93301,
        93900, 94280, 94680, 96310, 96400, 96800, 97350, 98090, 98400, 98790,
        99370, 99760, 99765 };
    const std::set<int> pri3s(pri3s_ids, boost::end(pri3s_ids));
    
    const int coast_ids[] = {
        17000, 17280, 17290, 27080, 27150, 27230, 27240, 27270, 27410, 27490,
        27500, 29950, 30000, 34080, 34120, 34130, 35850, 35860, 36150, 36200,
        38110, 39040, 39100, 39170, 39350, 41090, 41100, 41110, 41750, 41760,
        41770, 41772, 42160, 43270, 43340, 43350, 43900, 44080, 44320, 44560,
        44600, 44610, 44630, 44640, 45870, 45880, 45900, 47190, 47200, 47210,
        47260, 47300, 48120, 48300, 48330, 50460, 50500, 50540, 50541, 50542,
        50543, 50544, 50560, 50700, 52530, 52535, 52800, 52860, 56420, 56440,
        57710, 57720, 57740, 57750, 57760, 57770, 59100, 59110, 59580, 59610,
        59800, 59810, 60830, 60945, 60950, 60990, 61040, 61060, 61150, 61170,
        61180, 62270, 62300, 62310, 62480, 62490, 62500, 62640, 62650, 62660,
        64250, 64260, 64330, 65300, 65310, 65340, 65370, 65450, 65720, 65940,
        65950, 71540, 71550, 71560, 71650, 71780, 71850, 71980, 71990, 75220,
        75300, 75350, 75410, 75550, 75600, 75700, 76300, 76310, 76320, 76330,
        76450, 76500, 76530, 76600, 76750, 76810, 76820, 76850, 76900, 76920,
        76923, 76925, 76926, 76928, 76930, 76931, 76932, 76933, 76940, 76941,
        76942, 76943, 76944, 76945, 76946, 76947, 76948, 76949, 76950, 76951,
        76970, 76971, 76974, 76978, 76980, 76985, 77002, 77005, 77006, 77020,
        77021, 77022, 77023, 77024, 80100, 80101, 80102, 80300, 80610, 80740,
        80900, 80950, 82240, 82250, 82260, 82270, 82280, 82290, 82400, 82410,
        83120, 83550, 83700, 83710, 85030, 85040, 85230, 85240, 85380, 85450,
        85460, 85560, 85700, 85780, 85840, 85890, 85891, 85900, 85910, 85950,
        86150, 86200, 86230, 86260, 86500, 86520, 86600, 86740, 86750, 86760,
        86780, 87100, 87110, 87111, 87120, 87350, 87400, 87410, 87420, 87640,
        88580, 88680, 88690, 90280, 90400, 90440, 90450, 90490, 90500, 90700,
        90720, 90721, 90760, 90800, 90900, 91190, 92700, 92750, 93000, 94250,
        94260, 94280, 94350, 94450, 94500, 94600, 94680, 94700, 95800, 96300,
        96310, 96400, 96550, 98090, 98360, 98400, 98550, 98580, 98700, 98790,
        98800, 99710, 99720, 99735, 99737, 99754, 99765, 99790, 99821, 99950,
        99970 };
    const std::set<int> coast(coast_ids, boost::end(coast_ids));

    const int N_FOREIGN = 13;
    const int foreignId[N_FOREIGN] = {
        99986, 99990, 202000, 203600, 208000, 210400, 211000,
        220600, 222200, 230800, 241800, 250000, 280700 };
    const QString foreignName[N_FOREIGN] = {
        " ", " ", "NORDLAND", "NORDLAND", "FINNMARK", "NORDLAND", "NORDLAND",
        "NORD-TRØNDELAG", "NORD-TRØNDELAG", "SØR-TRØNDELAG", "ØSTFOLD",
        "ØSTFOLD", "FINNMARK" };
    
    const char stinfosys_SQL[] =
        "SELECT DISTINCT s.stationid, m.name,"
        "       (SELECT mm.name FROM municip mm"
        "        WHERE mm.municipid BETWEEN 1 AND 100"
        "              AND ((m.municipid NOT IN (2300,2800) AND mm.municipid = m.municipid / 100)"
        "                   OR (m.municipid IN (2300,2800) AND mm.municipid = m.municipid)))"
        " FROM station s, municip m"
        " WHERE s.stationid BETWEEN 60 AND 99999"
        "   AND s.municipid = m.municipid"
        " ORDER by s.stationid";
    QSqlQuery query(QSqlDatabase::database(QSQLNAME_REMOTE));
    if (not query.exec(stinfosys_SQL)) {
        METLIBS_LOG_ERROR("query to stinfosys failed: " << query.lastError().text());
        return false;
    }
    typedef std::map<int,countyInfo> cList_t;
    cList_t cList;
    METLIBS_LOG_INFO("got " << query.size() << " station city/county names from stinfosys");
    while( query.next() ) {
        countyInfo cInfo;
        cInfo.stnr    = query.value(0).toInt();
	cInfo.municip = query.value(1).toString();
        cInfo.county  = query.value(2).toString();

        if( cInfo.county == "SVALBARD" || cInfo.county == "JAN MAYEN" )
            cInfo.county = "ISHAVET";

        if( webs.find(cInfo.stnr) != webs.end() )
            cInfo.web = "WEB";
        if( pri3s.find(cInfo.stnr) != pri3s.end() )
            cInfo.pri = "PRI3";
        else if( pri2s.find(cInfo.stnr) != pri2s.end() )
            cInfo.pri = "PRI2";
        else if( pri1s.find(cInfo.stnr) != pri1s.end() )
            cInfo.pri = "PRI1";

        if (coast.find(cInfo.stnr) != coast.end())
            cInfo.ki = "K";
        else
            cInfo.ki = "I";
          
        cList[cInfo.stnr] = cInfo;
    }
    
    // some additional stations
    for( int i=0; i<N_FOREIGN; i++ ) {
        countyInfo cInfo;
        cInfo.stnr   = foreignId[i];
        cInfo.county = foreignName[i];
        cInfo.ki = "I";
        cList[cInfo.stnr] = cInfo;
    }

    listStat.clear();
    const std::list<kvalobs::kvStation> slist = KvMetaDataBuffer::instance()->allStations();
    BOOST_FOREACH(const kvalobs::kvStation& st, slist) {
        cList_t::const_iterator cit = cList.find(st.stationID());
        if( cit == cList.end() ) {
            if( st.stationID() < 100000 )
                METLIBS_LOG_ERROR("station " << st.stationID() << " from kvalobs' station table not found in stinfosys");
            continue;
        }
        const countyInfo& ci = cit->second;

        listStat_t ls;
        ls.name        = st.name();
        ls.stationid   = st.stationID();
        ls.altitude    = st.height();
        ls.environment = st.environmentid();
        ls.fylke       = ci.county.toStdString();
        ls.kommune     = ci.municip.toStdString();
        ls.wmonr       = ci.web.toStdString();
        ls.pri         = ci.pri.toStdString();
        ls.coast       = ci.ki == "K";
        listStat.push_back(ls);
    }
    METLIBS_LOG_INFO("stationliste hentet fra stinfosys");
    writeToStationFile();
    return true;
}

StInfoSysBuffer::StInfoSysBuffer(miutil::conf::ConfSection *conf)
{
    connect2stinfosys(conf);
}

StInfoSysBuffer::~StInfoSysBuffer()
{
    QSqlDatabase::removeDatabase(QSQLNAME_REMOTE);
}

bool StInfoSysBuffer::isConnected()
{
    QSqlDatabase db = QSqlDatabase::database("stinfosys");
    return db.isOpen();
}

void StInfoSysBuffer::readStationInfo()
{
    if (not readFromStInfoSys())
        StationInfoBuffer::readStationInfo();
}
