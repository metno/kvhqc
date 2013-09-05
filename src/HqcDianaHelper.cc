
#include "HqcDianaHelper.hh"

#include "dianashowdialog.h"
#include "KvMetaDataBuffer.hh"

#include <qUtilities/ClientButton.h>
#include <qUtilities/miMessage.h>
#include <qUtilities/QLetterCommands.h>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <sstream>

#define NDEBUG
#include "debug.hh"

HqcDianaHelper::HqcDianaHelper(DianaShowDialog* dshdlg, ClientButton* pluginB)
    : mDianaConfigDialog(dshdlg)
    , mClientButton(pluginB)
    , mDianaConnected(false)
    , mDianaNeedsHqcInit(true)
    , mCountSameTime(0)
    , mEnabled(true)
{
    mClientButton->useLabel(true);
    mClientButton->connectToServer();

#ifdef METLIBS_BEFORE_4_9_5
    connect(mClientButton, SIGNAL(receivedMessage(miMessage&)), SLOT(processLetterOld(miMessage&)));
#else
    connect(mClientButton, SIGNAL(receivedMessage(const miMessage&)), SLOT(processLetter(const miMessage&)));
#endif
    connect(mClientButton, SIGNAL(addressListChanged()), this, SLOT(handleAddressListChanged()));
    connect(mClientButton, SIGNAL(connectionClosed()),   this, SLOT(handleConnectionClosed()));

    updateDianaParameters();
}

void HqcDianaHelper::setEnabled(bool enabled)
{
    if (enabled == mEnabled)
        return;

    mEnabled = enabled;
    if (mEnabled and mDianaConnected) {
        mDianaNeedsHqcInit = true;
        sendTimes();
        sendTime();
    }
}

void HqcDianaHelper::handleAddressListChanged()
{
    LOG_SCOPE("HqcDianaHelper");
    const bool dc = (mClientButton->clientTypeExist("Diana"));
    if (dc == mDianaConnected)
        return;

    mDianaConnected = dc;
    if (mDianaConnected) {
        mDianaNeedsHqcInit = true;
        sendTimes();
        sendTime();
    }
    /*emit*/ connectedToDiana(mDianaConnected);
}

void HqcDianaHelper::handleConnectionClosed()
{
    LOG_SCOPE("HqcDianaHelper");
    mDianaConnected = false;
    /*emit*/ connectedToDiana(false);
}

void HqcDianaHelper::sendTimes(const std::set<timeutil::ptime>& allTimes)
{
    LOG_SCOPE("HqcDianaHelper");

    mAllTimes = allTimes;
    sendTimes();
    if (mEnabled and not isKnownTime(mDianaTime)) {
        LOG4SCOPE_DEBUG(DBG1(mDianaTime));
        if (not mAllTimes.empty()) {
            mDianaTime = *mAllTimes.begin();
            LOG4SCOPE_DEBUG(DBG1(mDianaTime));
            sendTime();
        } else {
            mDianaTime = timeutil::ptime();
            LOG4SCOPE_DEBUG(DBG1(mDianaTime));
        }
    }
}

void HqcDianaHelper::sendTimes()
{
    LOG_SCOPE("HqcDianaHelper");
    if (not mEnabled or not mDianaConnected or mAllTimes.empty())
        return;

    miMessage m;
    m.command= qmstrings::settime;
    m.commondesc = "datatype";
    m.common = "obs";
    m.description= "time";
    BOOST_FOREACH(const timeutil::ptime& t, mAllTimes)
        m.data.push_back(timeutil::to_iso_extended_string(t));
    LOG4SCOPE_DEBUG(DBG1(m.content()));
    mClientButton->sendMessage(m);
}

#ifdef METLIBS_BEFORE_4_9_5
void HqcDianaHelper::processLetterOld(miMessage& letter)
{
    processLetter(letter);
}
#endif

void HqcDianaHelper::processLetter(const miMessage& letter)
{
    LOG_SCOPE("HqcDianaHelper");
    LOG4SCOPE_DEBUG(DBG1(letter.command));
    if (letter.command == qmstrings::newclient) {
        std::vector<std::string> desc, valu;
        const std::string cd(letter.commondesc), co(letter.common);
        boost::split(desc, cd, boost::is_any_of(":"));
        boost::split(valu, co, boost::is_any_of(":"));
        for(std::vector<std::string>::const_iterator itD=desc.begin(), itC=valu.begin(); itD != desc.end(); ++itC, ++itD) {
            if( *itD == "type" && *itC == "Diana" ) {
                handleAddressListChanged();
                return;
            }
        }
    } else if (letter.command == qmstrings::station and mEnabled) {
        if (letter.commondesc == "name,time") {
            QStringList name_time = QString::fromStdString(letter.common).split(',');
            bool ok = false;
            const int stationid = name_time[0].toInt(&ok);
            if (not ok) {
                std::cerr << "Unable to parse first element as stationid: '" << letter.common << "'" << std::endl;
                return;
            }
            /*emit*/ receivedStation(stationid);
            handleDianaTime(name_time[1].toStdString());
        }
    } else if (letter.command == qmstrings::timechanged and mEnabled) {
        handleDianaTime(letter.common);
    } else {
        LOG4HQC_INFO("HqcDianaHelper", "Diana sent a command that HQC does not understand:\n"
                     << "====================\n" << letter.content() << "====================");
    }
}

void HqcDianaHelper::handleDianaTime(const std::string& time_txt)
{
    if (not mEnabled)
        return;
    timeutil::ptime time = timeutil::from_iso_extended_string(time_txt);
    if (not isKnownTime(time)) {
        std::cerr << "Invalid/unknown time from diana: '" << time_txt << "'" << std::endl;
        sendTimes();
        sendTime();
        /*emit*/ receivedTime(mDianaTime);
    } else if (time != mDianaTime) {
        mDianaTime = time;
        /*emit*/ receivedTime(mDianaTime);
    } else {
        LOG4SCOPE_DEBUG(DBG1(mDianaTime));
    }
}

void HqcDianaHelper::applyQuickMenu()
{
    LOG_SCOPE("HqcDianaHelper");
    if (not mEnabled)
        return;
#if 0 // FIXME this does not actually show the data
    miMessage m;
    m.command = qmstrings::apply_quickmenu;
    m.data.push_back("hqc_qmenu");
    m.data.push_back("hqc_obs_...");
    LOG4SCOPE_DEBUG(DBG1(m.content()));
    mClientButton->sendMessage(m);
#endif
}

void HqcDianaHelper::sendStation(int stnr)
{
    LOG_SCOPE("HqcDianaHelper");
    if (not mEnabled or not mDianaConnected)
        return;
    miMessage m;
    m.command = qmstrings::station;
    m.common  = boost::lexical_cast<std::string>(stnr);
    LOG4SCOPE_DEBUG(DBG1(m.content()));
    mClientButton->sendMessage(m);
}

void HqcDianaHelper::sendTime(const timeutil::ptime& time)
{
    DBG(DBG1(time) << DBG1(mDianaTime));
    if (not mEnabled or not mDianaConnected or mDianaTime == time or not isKnownTime(time))
        return;
    mDianaTime = time;
    sendTime();
}
    
void HqcDianaHelper::sendTime()
{
    LOG4SCOPE_DEBUG(DBG1(mDianaTime));
    if (not mEnabled or not mDianaConnected or not isKnownTime(mDianaTime))
        return;

    miMessage m;
    m.command = qmstrings::settime;
    m.commondesc = "time";
    m.common = timeutil::to_iso_extended_string(mDianaTime);
    LOG4SCOPE_DEBUG(DBG1(m.content()));
    mClientButton->sendMessage(m);
}

bool HqcDianaHelper::isKnownTime(const timeutil::ptime& time) const
{
    return (not time.is_not_a_date_time() and mAllTimes.find(time) != mAllTimes.end());
}

void HqcDianaHelper::sendObservations(const model::KvalobsDataList& datalist,
                                      const std::vector<modDatl>& modeldatalist,
                                      const std::vector<int>& selectedParameters)
{
    LOG_SCOPE("HqcDianaHelper");
    if (not mEnabled or not mDianaConnected or selectedParameters.empty() or mDianaTime.is_not_a_date_time())
        return;
    
    std::ostringstream synopDescription;
    synopDescription << "id,St.type,auto,lon,lat";
    BOOST_FOREACH(int pid, selectedParameters) {
        SendPars_t::const_iterator it = mSendPars.find(pid);
        if (it != mSendPars.end())
            synopDescription << ',' << it->second.dianaName;
    }
    
    std::vector<std::string> synopData;
    
    int prStnr = 0;
    LOG4SCOPE_DEBUG(DBG1(mDianaNeedsHqcInit));
    BOOST_FOREACH(const model::KvalobsData& d, datalist) {
        if (d.otime() != mDianaTime or d.stnr() == prStnr)
            continue;
        try {
            const kvalobs::kvStation& station = KvMetaDataBuffer::instance()->findStation(d.stnr());

            std::ostringstream synop;
            synop << d.stnr() << ',';

            const std::string stationType = hqcType(d.stnr(), station.environmentid());
            if (stationType == "AA" or stationType == "VM" or stationType == "none")
                synop << "none";
            else if (stationType.at(0) == 'P')
                synop << 'P';
            else
                synop << stationType.at(1);
            synop << ',';

            std::string isAuto = "x";
            if (stationType.at(0) == 'A')
                isAuto = "a";
            else if (stationType.at(0) == 'N' || stationType.at(0) == 'P')
                isAuto = "n";
            synop << isAuto << ',';
            
            synop << (boost::format("%1$.4f,%2$.4f") % station.lon() % station.lat()).str();
            BOOST_FOREACH(const int pid, selectedParameters) {
                SendPars_t::const_iterator it = mSendPars.find(pid);
                if (it == mSendPars.end())
                    continue;
                const SendPar& sp = it->second;

                double corr = d.corr(pid);
                if (sp.what != SendPar::CORRECTED) {
                    BOOST_FOREACH(const modDatl& modelV, modeldatalist) {
                        if (modelV.stnr == d.stnr() and modelV.otime == d.otime()) {
                            const double modelValue = modelV.orig[pid];
                            if (sp.what == SendPar::MODEL)
                                corr = modelValue;
                            else if (sp.what == SendPar::DIFF)
                                corr = corr - modelValue;
                            else if (sp.what == SendPar::PROPORTION)
                                corr = (abs(modelValue) > 0.00001) ? corr/modelValue : -32767;
                            break;
                        }
                    }
                }
                if(corr<-999){
                    synop << ",-32767";
                } else {
                    corr = dianaValue(pid, sp.what == SendPar::MODEL, corr, d.corr(1/*AA*/));
                    const int flag = d.flag(pid);
                    const int shFl1 = flag/10000;
                    const int shFl2 = flag%10000/1000;
                    const int shFl3 = flag%1000/100;
                    const int shFl4 = flag%100/10;
                    const int shFl5 = flag%10;
                    int maxFlag = shFl1 >shFl2 ? shFl1 : shFl2;
                    maxFlag = shFl3 > maxFlag ? shFl3 : maxFlag;
                    maxFlag = shFl4 > maxFlag ? shFl4 : maxFlag;
                    const std::string flagstr = (boost::format("%1$05d") % flag).str();
                    std::string colorstr;
                    if (maxFlag == 0)
                        colorstr = ";0:0:0";
                    if (maxFlag == 1 or (shFl5 != 0 && shFl5 != 9))
                        colorstr = ";0:255:0";
                    else if (maxFlag >= 2 && maxFlag <= 5)
                        colorstr = ";255:175:0";
                    else if (maxFlag >= 6)
                        colorstr = ";255:0:0";

                    synop << ',' << corr << ';' << flagstr << colorstr;
                }
            }
            synopData.push_back(synop.str());
            prStnr = d.stnr();
        } catch (std::runtime_error& e) {
            // unknown station, ignore
            LOG4SCOPE_DEBUG(DBG1(d.stnr()));
        }
    }

    if (not synopData.empty()) {
        miMessage pLetter;
        pLetter.command = mDianaNeedsHqcInit ? qmstrings::init_HQC_params : qmstrings::update_HQC_params;
        pLetter.commondesc = "time,plottype";
        pLetter.common = timeutil::to_iso_extended_string(mDianaTime) + ",synop";

        pLetter.description = synopDescription.str();
#ifdef METLIBS_BEFORE_4_9_5
        pLetter.data = std::vector<miutil::miString>(synopData.begin(), synopData.end());
#else
        pLetter.data = synopData;
#endif
        LOG4SCOPE_DEBUG(DBG1(pLetter.content()));
        mClientButton->sendMessage(pLetter);

        if (mDianaNeedsHqcInit)
            sendTimes();
        mDianaNeedsHqcInit = false;
    }
}

void HqcDianaHelper::sendSelectedParam(int paramId)
{
    LOG_SCOPE("HqcDianaHelper");

    if (not mEnabled or not mDianaConnected)
        return;
    SendPars_t::const_iterator it = mSendPars.find(paramId);
    if (it == mSendPars.end()) {
        std::cerr << "No such diana parameter: " << paramId << std::endl;
        return;
    }

    miMessage m;
    m.command = qmstrings::select_HQC_param;
    m.commondesc = "diParam";
    m.common     = it->second.dianaName;
    LOG4SCOPE_DEBUG(DBG1(m.content()));
    mClientButton->sendMessage(m);
}

// Help function to translate from kvalobs parameter value to diana parameter value
double HqcDianaHelper::dianaValue(int parNo, bool isModel, double qVal, double aa)
{
    double dVal;
    if ( parNo == 273 ) {
        if ( qVal <= 5000 )
            dVal = int(qVal)/100;
        else if ( qVal <= 30000 )
            dVal = int(qVal)/1000 + 50;
        else if ( qVal < 75000 )
            dVal = int(qVal)/5000 + 74;
        else
            dVal = 89;
        return dVal;
    }
    else if ( parNo == 55 ) {
        if ( qVal < 50 )
            dVal = 0;
        else if ( qVal < 100 )
            dVal = 1;
        else if ( qVal < 200 )
            dVal = 2;
        else if ( qVal < 300 )
            dVal = 3;
        else if ( qVal < 600 )
            dVal = 4;
        else if ( qVal < 1000 )
            dVal = 5;
        else if ( qVal < 1500 )
            dVal = 6;
        else if ( qVal < 2000 )
            dVal = 7;
        else if ( qVal < 2500 )
            dVal = 8;
        else
            dVal = 9;
        return dVal;
    }
    else if ( parNo == 177 ) {
        if ( aa >= 5.0 && !isModel ) {
            dVal = -qVal;
        }
        else {
            dVal = qVal;
        }
        return dVal;
    }
    //  else if ( parNo > 80 && parNo < 95 ) {
    //    dVal = 1.94384*qVal;
    //    return dVal;
    //  }
    else
        return qVal;
}

std::string HqcDianaHelper::hqcType(int typeId, int env)
{
    std::string hqct = "none";
    if ( env == 8 ) {
        if ( typeId == 3 || typeId == 330 || typeId == 342 )
            hqct = "AA";
        else if ( typeId == 1 || typeId == 6 || typeId == 312 || typeId == 412 )
            hqct = "VS";
        else if ( typeId == 306 )
            hqct = "VM";
    }
    else if ( env == 2 ) {
        if ( typeId == 3 )
            hqct = "AL";
    }
    else if ( env == 12 ) {
        if ( typeId == 3 )
            hqct = "AV";
    }
    else if ( env == 3 ) {
        if ( typeId == 3 || typeId == 410 )
            hqct = "AP";
        else if ( typeId == 412 )
            hqct = "VK";
    }
    else if ( env == 1 ) {
        if ( typeId == 310 || typeId == 311 || typeId == 410 || typeId == 1 )
            hqct = "AF";
        else if ( typeId == 2 )
            hqct = "FM";
        else if ( typeId == 1 || typeId == 6 || typeId == 312 || typeId == 412 )
            hqct = "VS";
    }
    else if ( env == 5 ) {
        if ( typeId == 6 || typeId == 430 )
            hqct = "AE";
        else if ( typeId == 11 )
            hqct = "MP";
    }
    else if ( env == 7 ) {
        if ( typeId == 11 )
            hqct = "MV";
    }
    else if ( env == 4 ) {
        if ( typeId == 11 )
            hqct = "MM";
    }
    else if ( env == 6 ) {
        if ( typeId == 11 )
            hqct = "MS";
    }
    else if ( env == 9 ) {
        if ( typeId == 302 || typeId == 303 )
            hqct = "NS";
        else if ( typeId == 402 )
            hqct = "ND";
        else if ( typeId == 4 || typeId == 405 )
            hqct = "P ";
    }
    else if ( env == 10 ) {
        if ( typeId == 402 )
            hqct = "NO";
    }
    else if ( env == 11 ) {
        if ( typeId == 309 )
            hqct = "VT";
    }
    return hqct;
}

void HqcDianaHelper::updateDianaParameters()
{
    LOG_SCOPE("HqcDianaHelper");
    mSendPars.clear();

    const char* item = "TTT";
    if (mDianaConfigDialog->tameType->isChecked())
        mSendPars.insert(std::make_pair(212/*TAM*/, SendPar(item)));
    else if (mDianaConfigDialog->tamoType->isChecked())
        mSendPars.insert(std::make_pair(211/*TA*/, SendPar(item, SendPar::MODEL)));
    else if (mDianaConfigDialog->tadiType->isChecked())
        mSendPars.insert(std::make_pair(211/*TA*/, SendPar(item, SendPar::DIFF)));
    else
        mSendPars.insert(std::make_pair(211, SendPar(item)));

    item = "TdTdTd";
    if (mDianaConfigDialog->uuType->isChecked())
        mSendPars.insert(std::make_pair(262/*UU*/, SendPar(item)));
    else if ( mDianaConfigDialog->uumoType->isChecked())
        mSendPars.insert(std::make_pair(262/*UU*/, SendPar(item, SendPar::MODEL)));
    else if (mDianaConfigDialog->uumeType->isChecked())
        mSendPars.insert(std::make_pair(263/*UM*/, SendPar(item)));
    else if (mDianaConfigDialog->uumiType->isChecked())
        mSendPars.insert(std::make_pair(264/*UN*/, SendPar(item)));
    else if (mDianaConfigDialog->uumaType->isChecked())
        mSendPars.insert(std::make_pair(265/*UX*/, SendPar(item)));
    else
        mSendPars.insert(std::make_pair(217/*TD*/, SendPar(item)));

    item = "PPPP";
    if (mDianaConfigDialog->poType->isChecked())
        mSendPars.insert(std::make_pair(173/*PO*/, SendPar(item)));
    else if (mDianaConfigDialog->prmoType->isChecked())
        mSendPars.insert(std::make_pair(173/*PO*/, SendPar(item, SendPar::MODEL)));
    else if (mDianaConfigDialog->podiType->isChecked())
        mSendPars.insert(std::make_pair(173/*PO*/, SendPar(item, SendPar::DIFF)));
    else if (mDianaConfigDialog->pomeType->isChecked())
        mSendPars.insert(std::make_pair(174/*POM*/, SendPar(item))); 
    else if (mDianaConfigDialog->pomiType->isChecked())
        mSendPars.insert(std::make_pair(175/*PON*/, SendPar(item)));
    else if (mDianaConfigDialog->pomaType->isChecked())
        mSendPars.insert(std::make_pair(176/*POX*/, SendPar(item)));
    else if (mDianaConfigDialog->phType->isChecked())
        mSendPars.insert(std::make_pair(172/*PH*/, SendPar(item)));
    else
        mSendPars.insert(std::make_pair(178/*PR*/, SendPar(item)));

    item = "ppp";
    if (mDianaConfigDialog->ppmoType->isChecked())
        mSendPars.insert(std::make_pair(177/*PP*/, SendPar(item, SendPar::MODEL)));
    else
        mSendPars.insert(std::make_pair(177/*PP*/, SendPar(item)));

    item = "RRR";
    if (mDianaConfigDialog->rrmoType->isChecked())
        mSendPars.insert(std::make_pair(109/*RR_12*/, SendPar(item, SendPar::MODEL)));
    else if (mDianaConfigDialog->rr1Type->isChecked())
        mSendPars.insert(std::make_pair(106/*RR_1*/, SendPar(item)));
    else if (mDianaConfigDialog->rr6Type->isChecked())
        mSendPars.insert(std::make_pair(108/*RR_6*/, SendPar(item)));
    else if (mDianaConfigDialog->rr24Type->isChecked())
        mSendPars.insert(std::make_pair(110/*RR_24*/, SendPar(item)));
    else if (mDianaConfigDialog->rr24moType->isChecked())
        mSendPars.insert(std::make_pair(110/*RR_24*/, SendPar(item, SendPar::MODEL)));
    else if (mDianaConfigDialog->rrprType->isChecked())
        mSendPars.insert(std::make_pair(109/*RR_12*/, SendPar(item, SendPar::PROPORTION)));
    else
        mSendPars.insert(std::make_pair(109/*RR_12*/, SendPar(item)));

    item = "TxTn";
    if (mDianaConfigDialog->tx12Type->isChecked())
        mSendPars.insert(std::make_pair(216/*TAX_12*/, SendPar(item)));
    else if (mDianaConfigDialog->tnType->isChecked())
        mSendPars.insert(std::make_pair(213/*TAN*/, SendPar(item)));
    else if (mDianaConfigDialog->txType->isChecked())
        mSendPars.insert(std::make_pair(215/*TAX*/, SendPar(item)));
    else
        mSendPars.insert(std::make_pair(214/*TAN_12*/, SendPar(item)));

    item = "dd";
    if (mDianaConfigDialog->ddmoType->isChecked())
        mSendPars.insert(std::make_pair(61/*DD*/, SendPar(item, SendPar::MODEL)));
    else
        mSendPars.insert(std::make_pair(61/*DD*/, SendPar(item)));

    item = "ff";
    if ( mDianaConfigDialog->ffmoType->isChecked())
        mSendPars.insert(std::make_pair(81/*FF*/, SendPar(item, SendPar::MODEL)));
    else
        mSendPars.insert(std::make_pair(81/*FF*/, SendPar(item)));

    item = "fxfx";
    if (mDianaConfigDialog->fx01Type->isChecked())
        mSendPars.insert(std::make_pair(87/*FX_1*/, SendPar(item)));
    else
        mSendPars.insert(std::make_pair(86/*FX*/, SendPar(item)));

    item = "ff_911";
    if (mDianaConfigDialog->fg01Type->isChecked())
        mSendPars.insert(std::make_pair(90/*FG_1*/, SendPar(item)));
    else if (mDianaConfigDialog->fg10Type->isChecked())
        mSendPars.insert(std::make_pair(84/*FG_10*/, SendPar(item)));
    else
        mSendPars.insert(std::make_pair(83/*FG*/, SendPar(item)));

    item = "sss";
    if (mDianaConfigDialog->sdType->isChecked())
        mSendPars.insert(std::make_pair(18/*SD*/, SendPar(item)));
    else if (mDianaConfigDialog->emType->isChecked())
        mSendPars.insert(std::make_pair(7/*EM*/, SendPar(item)));
    else
        mSendPars.insert(std::make_pair(112/*SA*/, SendPar(item)));

    mSendPars.insert(std::make_pair( 55/*HL*/,     SendPar("h")));
    mSendPars.insert(std::make_pair( 15/*NN*/,     SendPar("N")));
    mSendPars.insert(std::make_pair(  1/*AA*/,     SendPar("a")));
    mSendPars.insert(std::make_pair( 41/*WW*/,     SendPar("ww")));
    mSendPars.insert(std::make_pair( 14/*NH*/,     SendPar("Nh")));
    mSendPars.insert(std::make_pair( 23/*CL*/,     SendPar("Cl")));
    mSendPars.insert(std::make_pair( 24/*CM*/,     SendPar("Cm")));
    mSendPars.insert(std::make_pair( 22/*CH*/,     SendPar("Ch")));
    mSendPars.insert(std::make_pair(242/*TW*/,     SendPar("TwTwTw")));
    mSendPars.insert(std::make_pair(273/*VV*/,     SendPar("VV")));
    mSendPars.insert(std::make_pair( 42/*W1*/,     SendPar("W1")));
    mSendPars.insert(std::make_pair( 43/*W2*/,     SendPar("W2")));
    mSendPars.insert(std::make_pair(134/*HWA*/,    SendPar("HwaHwa")));
    mSendPars.insert(std::make_pair(154/*PWA*/,    SendPar("PwaPwa")));
    mSendPars.insert(std::make_pair( 65/*DW1*/,    SendPar("dw1dw1")));
    mSendPars.insert(std::make_pair(152/*PW1*/,    SendPar("Pw1Pw1")));
    mSendPars.insert(std::make_pair(132/*HW1*/,    SendPar("Hw1Hw1")));
    mSendPars.insert(std::make_pair( 19/*SG*/,     SendPar("s")));
    mSendPars.insert(std::make_pair(403/*MDIR*/,   SendPar("ds")));
    mSendPars.insert(std::make_pair(404/*MSPEED*/, SendPar("vs")));
}
