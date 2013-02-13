
#ifndef HqcDianaHelper_hh
#define HqcDianaHelper_hh 1

#include "hqcdefs.h"
#include "KvalobsData.h"
#include "timeutil.hh"

#include <QtCore/QMap>
#include <QtCore/QObject>

#include <set>
#include <map>

class DianaShowDialog;
class ClientButton;
class miMessage;

class HqcDianaHelper : public QObject
{ Q_OBJECT;
public:
    HqcDianaHelper(DianaShowDialog* dshdlg, ClientButton* pluginB);

    bool isConnected() const
        { return mDianaConnected; }
    const timeutil::ptime& dianaTime() const
        { return mDianaTime; }

    void setFirstObs()
        { mDianaNeedsHqcInit = true; }

    void applyQuickMenu();
    void sendTimes(const std::set<timeutil::ptime>& allTimes);
    void sendTime(const timeutil::ptime& time);
    void sendStation(int stationID);
    void sendObservations(const model::KvalobsDataList& datalist,
                          const std::vector<modDatl>& modeldatalist,
                          const std::vector<int>& selectedParameters);
    void sendSelectedParam(int paramId);

    void updateDianaParameters();

private Q_SLOTS:
    void handleAddressListChanged();
    void handleConnectionClosed();

#ifdef METLIBS_BEFORE_4_9_5
    void processLetterOld(miMessage& letter);
#endif
    void processLetter(const miMessage& letter);

Q_SIGNALS:
    void connectedToDiana(bool);
    void receivedStation(int stationid);
    void receivedTime(const timeutil::ptime& time);

private:
    double dianaValue(int parNo, bool isModel, double qVal, double aa);
    std::string hqcType(int typeId, int env);
    void sendTimes();
    void sendTime();
    bool isKnownTime(const timeutil::ptime& time) const;
    void handleDianaTime(const std::string& time_txt);

private:
    DianaShowDialog* mDianaConfigDialog;
    ClientButton* mClientButton;

    bool mDianaConnected;
    bool mDianaNeedsHqcInit;
    timeutil::ptime mDianaTime;
    int mCountSameTime;
    std::set<timeutil::ptime> mAllTimes;

    struct SendPar {
        std::string dianaName;
        enum What { CORRECTED, MODEL, DIFF, PROPORTION };
        What what;
        SendPar(const std::string& dn, What w = CORRECTED)
            : dianaName(dn), what(w) { }
    };
    typedef std::map<int, SendPar> SendPars_t;
    SendPars_t mSendPars;
};

#endif // HqcDianaHelper_hh
