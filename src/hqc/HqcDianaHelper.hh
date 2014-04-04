
#ifndef HqcDianaHelper_hh
#define HqcDianaHelper_hh 1

#include "common/DataView.hh"

#include <QtCore/QMap>
#include <QtCore/QObject>

#include <deque>
#include <map>
#include <set>
#include <vector>

class ClientButton;
class miMessage;

class HqcDianaHelper : public QObject, public DataView
{   Q_OBJECT;
public:
  HqcDianaHelper(ClientButton* pluginB);

  void setSensorsAndTimes(const Sensors_t& sensors, const TimeSpan& limits);

  bool isConnected() const
    { return mDianaConnected; }

  void setEnabled(bool enabled);

  void applyQuickMenu();

  void updateDianaParameters();

  void navigateTo(const SensorTime&);

private Q_SLOTS:
  void handleAddressListChanged();
  void handleConnectionClosed();

  void processLetter(const miMessage& letter);

Q_SIGNALS:
  void connectedToDiana(bool);

protected:
  virtual void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

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
  void sendStation();
  void sendObservations();
  void sendSelectedParam();
  void sendMessage(miMessage& message);

  std::string synopStart(int stationId);
  std::string synopValue(const SensorTime& st, const SendPar& sp, bool& hasData);
  double dianaValue(int parNo, bool isModel, double qVal, double aa);
  std::string hqcType(int typeId, int env);

  void sendTimes();
  void sendTime();
  bool isKnownTime(const timeutil::ptime& time) const;
  bool switchToKvalobsStationId(int stationId);
  void handleDianaStationAndTime(int stationId, const std::string& time_txt);
  void handlePosition(float lon, float lat);

private:
  ClientButton* mClientButton;

  bool mDianaConnected;
  bool mDianaNeedsHqcInit;
  SensorTime mDianaSensorTime;
  int mCountSameTime;
  bool mEnabled;

  Sensors_t mSensors;
  TimeSpan mTimeLimits;
  std::set<timeutil::ptime> mAllTimes;

  SendPars_t mSendPars;

  typedef std::deque<timeutil::ptime> TimesAwaitingConfirmation_t;

  /*! diana sends back all times we set via "settime" with a
    "timechanged" message, but this is delayed quite a bit if diana
    needs to fetch data; therefore we keep a list of times we have
    sent but for which diana has not yet sent a "timechanged" message;
    when we then receive a "timechanged" message and it is at the
    beginning of this list (first few), we do not inform other views
    but instead delete the time from this list */
  TimesAwaitingConfirmation_t mTimesAwaitingConfirmation;
};

#endif // HqcDianaHelper_hh
