
#ifndef HqcDianaHelper_hh
#define HqcDianaHelper_hh 1

#include "DataView.hh"

#include <QtCore/QMap>
#include <QtCore/QObject>

#include <set>
#include <map>
#include <vector>

class DianaShowDialog;
class ClientButton;
class miMessage;

class HqcDianaHelper : public QObject, public DataView
{   Q_OBJECT;
public:
    HqcDianaHelper(DianaShowDialog* dshdlg, ClientButton* pluginB);

    void setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits);

    bool isConnected() const
        { return mDianaConnected; }

    void setEnabled(bool enabled);

    void applyQuickMenu();

    void updateDianaParameters();

    void navigateTo(const SensorTime&);

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
    struct SendPar {
        std::string dianaName;
        enum What { CORRECTED, MODEL, DIFF, PROPORTION };
        What what;
        SendPar(const std::string& dn, What w = CORRECTED)
            : dianaName(dn), what(w) { }
    };
    typedef std::map<int, SendPar> SendPars_t;

private:
    void sendTimes(const std::set<timeutil::ptime>& allTimes);
    void sendTime(const timeutil::ptime& time);
    void sendStation(int stationID);
    void sendObservations();
    void sendSelectedParam(int paramId);

    std::string synopStart(int stationId);
    std::string synopValue(const SensorTime& st, const SendPar& sp, bool& hasData);
    double dianaValue(int parNo, bool isModel, double qVal, double aa);
    std::string hqcType(int typeId, int env);

    void sendTimes();
    void sendTime();
    bool isKnownTime(const timeutil::ptime& time) const;
    void handleDianaStationAndTime(int stationId, const std::string& time_txt);

private:
    DianaShowDialog* mDianaConfigDialog;
    ClientButton* mClientButton;

    bool mDianaConnected;
    bool mDianaNeedsHqcInit;
    SensorTime mDianaSensorTime;
    int mCountSameTime;
    bool mEnabled;

    Sensors_t mSensors;
    TimeRange mTimeLimits;
    std::set<timeutil::ptime> mAllTimes;

    SendPars_t mSendPars;
};

#endif // HqcDianaHelper_hh
