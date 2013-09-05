
#include "StationInfoBuffer.hh"

#include "Functors.hh"
#include "KvMetaDataBuffer.hh"

#include <miconfparser/confexception.h>
#include <miconfparser/confsection.h>
#include <miconfparser/valelement.h>
#include <puTools/miString.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <boost/foreach.hpp>
#include <boost/range.hpp>

#include <iomanip>
#include <fstream>
#include <set>

#define NDEBUG
#include "debug.hh"

namespace /*anonymous*/ {

struct countyInfo {
    int stnr;
    QString name;
    QString county;
    QString municip;
    QString web;
    QString pri;
    QString ki;
};

} // anonymous namespace

StationInfoBuffer* StationInfoBuffer::sInstance = 0;

StationInfoBuffer::StationInfoBuffer()
{
    assert(not sInstance);
    sInstance = this;
}

StationInfoBuffer::~StationInfoBuffer()
{
    sInstance = 0;
}

/*!
  Read station info from the stinfosys database
*/
bool StationInfoBuffer::writeToStationFile()
{
    LOG_SCOPE("StationInfoBuffer");

    const std::string path = localCacheFileName();
    if (path.empty())
        return false;
    std::ofstream outf(path.c_str());
    if (not outf.is_open())
        return false;

    BOOST_FOREACH(const listStat_t& ls, listStat) {
        outf << std::setw(7)  << std::right << ls.stationid << " " << std::left
             << std::setw(31) << ls.fylke
             << std::setw(25) << ls.kommune
             << std::setw(3)  << ls.wmonr
             << std::setw(4)  << ls.pri
             << std::setw(1)  << (ls.coast ? 'K' : 'I')
             << std::endl;
    }
    outf.close();
    LOG4HQC_INFO("StationInfoBuffer", "station list written to local cache");
    return true;
}

/*!
  Read the station file, this must be done after the station table in the database is read
*/
bool StationInfoBuffer::readFromStationFile()
{
    LOG_SCOPE("StationInfoBuffer");
    const std::string path = localCacheFileName();
    if (path.empty())
        return false;

    QFile stations(QString::fromStdString(path));
    if (not stations.open(QIODevice::ReadOnly)) {
        LOG4HQC_INFO("StationInfoBuffer", "cannot open '" << path << "' for reading");
        return false;
    }
    QTextStream stationStream(&stations);
    int prevStnr = 0;
    while (not stationStream.atEnd()) {
        QString statLine = stationStream.readLine();
        int stnr = statLine.left(7).toInt();
        if (stnr == prevStnr)
            continue;

        try {
            const kvalobs::kvStation& st = KvMetaDataBuffer::instance()->findStation(stnr);
            listStat_t ls;
            ls.name        = st.name();
            ls.stationid   = st.stationID();
            ls.altitude    = st.height();
            ls.environment = st.environmentid();
            ls.wmonr       = st.wmonr();
            ls.fylke       = statLine.mid(8,30).stripWhiteSpace().toStdString();
            ls.kommune     = statLine.mid(39,24).stripWhiteSpace().toStdString();
            ls.pri         = statLine.mid(67,4).stripWhiteSpace().toStdString();
            ls.coast       = statLine.mid(71,1).stripWhiteSpace() == "K";
            listStat.push_back(ls);
        } catch (std::runtime_error& e) {
            // unknown station
        }
        prevStnr = stnr;
    }
    return true;
}

std::string StationInfoBuffer::localCacheFileName() const
{
    std::string path = miutil::from_c_str(getenv("HOME"));
    if (path.empty())
        return path;

    return path + "/.config/hqc_stations";
}

bool StationInfoBuffer::isConnected()
{
    const std::string path = localCacheFileName();
    if (path.empty())
        return false;

    QFile stations(QString::fromStdString(path));
    return stations.open(QIODevice::ReadOnly);
}

const listStat_l& StationInfoBuffer::getStationDetails()
{
    LOG_SCOPE("StationInfoBuffer");

    const timeutil::ptime now = timeutil::now();
    DBG(DBG1(now) << DBG1(mLastStationListUpdate));
    if (mLastStationListUpdate.is_not_a_date_time()
        or (now - mLastStationListUpdate).total_seconds() > 3600)
    {
        mLastStationListUpdate = now;
        readStationInfo();
    }
    return listStat;
}

void StationInfoBuffer::readStationInfo()
{
    readFromStationFile();
}
