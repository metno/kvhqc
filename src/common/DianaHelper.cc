
#include "DianaHelper.hh"

#include "KvMetaDataBuffer.hh"
#include "util/hqc_paths.hh"

#ifdef METLIBS_BEFORE_4_9_5
#define signals Q_SIGNALS
#define slots Q_SLOTS
#endif
#include <coserver/ClientButton.h>
#include <coserver/miMessage.h>
#include <coserver/QLetterCommands.h>

#include <boost/foreach.hpp>

#include <iomanip>
#include <sstream>

#define MILOGGER_CATEGORY "kvhqc.DianaHelper"
#include "util/HqcLogging.hh"

namespace {
const char ANNOTATION[] = "HQC";
const char IMG_STD_HQC[]  = "diana_img_hqc";
const char IMG_ICON_HQC[] = "diana_icon_hqc";
}

DianaHelper::DianaHelper(ClientButton* cb)
    : mDianaButton(cb)
    , mConnected(mDianaButton->clientTypeExist("Diana"))
{
#ifdef METLIBS_BEFORE_4_9_5
    connect(mDianaButton, SIGNAL(receivedMessage(miMessage&)), SLOT(processLetter(const miMessage&)));
#else
    connect(mDianaButton, SIGNAL(receivedMessage(const miMessage&)), SLOT(processLetter(const miMessage&)));
#endif
    connect(mDianaButton, SIGNAL(addressListChanged()), SLOT(processConnect()));
    connect(mDianaButton, SIGNAL(connectionClosed()), SLOT(cleanConnection()));
}

void DianaHelper::tryConnect()
{
    mDianaButton->connectToServer();
}

void DianaHelper::processConnect()
{
    const bool c = mDianaButton->clientTypeExist("Diana");
    if (c == mConnected)
        return;

    mConnected = c;
    if (mConnected) {
        const QString pStd = ::hqc::getPath(::hqc::IMAGEDIR) + "/" + IMG_STD_HQC  + ".png";
        METLIBS_LOG_DEBUG(LOGVAL(pStd.toStdString()));
        const QImage iStd(pStd);
        const QImage iIcon(::hqc::getPath(::hqc::IMAGEDIR) + "/" + IMG_ICON_HQC + ".png");
        
        sendImage(IMG_STD_HQC, iStd);
        sendImage(IMG_ICON_HQC, iIcon);
        
        miMessage m;
        m.command = qmstrings::showpositionname;
        m.description = "normal:selected:icon";
        m.data.push_back("true:true:true");
        mDianaButton->sendMessage(m);
    }
    /*emit*/ connection(mConnected);
}

void DianaHelper::cleanConnection()
{
    mConnected = false;
    /*emit*/ connection(mConnected);
}

void DianaHelper::processLetter(const miMessage& m)
{
    METLIBS_LOG_SCOPE();
    METLIBS_LOG_DEBUG(m.content());

    if (m.command == qmstrings::timechanged) {
        const timeutil::ptime newTime = timeutil::from_iso_extended_string(m.common);
        if (newTime != mDianaTime) {
            mDianaTime = newTime;
            /*emit*/ receivedTime(mDianaTime);
        }
    }
}

// send one image to diana (with name); copied from tseries/qtsMain.cc
void DianaHelper::sendImage(const std::string& name, const QImage& image)
{
    METLIBS_LOG_SCOPE();
    if (image.isNull()) {
        std::cerr << "image '" << name << "' is null" << std::endl;
        return;
    }
    if (not mConnected) {
        std::cerr << "diana not connected, cannot send image" << std::endl;
        return;
    }

    QByteArray *a = new QByteArray();
    QDataStream s(a, QIODevice::WriteOnly);
    s << image;
    
    miMessage m;
    m.command = qmstrings::addimage;
    m.description = "name:image";
    
    std::ostringstream ost;
    ost << name << ":";
    
    const int n = a->count();
    for (int i = 0; i < n; i++) {
        ost << std::setw(7) << int(a->data()[i]);
    }
    m.data.push_back(ost.str());
    
    mDianaButton->sendMessage(m);
    std::cerr << "sent image '" << name << '\'' << std::endl;
}

void DianaHelper::sendTimes(const std::vector<timeutil::ptime>& times)
{
    METLIBS_LOG_SCOPE();
    if (not mConnected or times.empty())
        return;

    miMessage m;
    m.command= qmstrings::settime;
    m.commondesc = "datatype";
    m.common = "obs";
    m.description= "time";
    BOOST_FOREACH(const timeutil::ptime& t, times)
        m.data.push_back(timeutil::to_iso_extended_string(t));
    mDianaButton->sendMessage(m);
}

void DianaHelper::sendTime(const timeutil::ptime& time)
{
    METLIBS_LOG_SCOPE();
    if (not mConnected or time == mDianaTime)
        return;

    mDianaTime = time;
    miMessage m2;
    m2.command = qmstrings::settime;
    m2.commondesc = "time";
    m2.common = timeutil::to_iso_extended_string(mDianaTime);
    mDianaButton->sendMessage(m2);
}

void DianaHelper::sendStations(const std::vector<int>& stations)
{
    METLIBS_LOG_SCOPE();
    if (not mConnected)
        return;

    const std::string annotation = ANNOTATION;
    if (stations.empty()) {
        miMessage m;
        m.command = qmstrings::hidepositions;
        m.description = ANNOTATION;
        mDianaButton->sendMessage(m);
    } else {
        miMessage m2;
        m2.command     = qmstrings::positions;
        m2.commondesc  = "dataset:image:icon:annotation:normal:selected";
        m2.common      =  annotation + ":" + std::string(IMG_STD_HQC) + ":" + std::string(IMG_ICON_HQC) + ":" + annotation + ":true:true";
        m2.description = "name:id:lat:lon";

        BOOST_FOREACH(int sid, stations) {
            try {
                const kvalobs::kvStation& s = KvMetaDataBuffer::instance()->findStation(sid);
                std::ostringstream o;
                o << s.name() << ':' << sid << ':' << s.lat() << ':' << s.lon();
                m2.data.push_back(o.str());
            } catch (std::exception&) {
            }
        }
        METLIBS_LOG_DEBUG(m2.content());
        mDianaButton->sendMessage(m2);

        miMessage m1;
        m1.command = qmstrings::showpositions;
        m1.description = ANNOTATION;
        mDianaButton->sendMessage(m1);
    }
}
