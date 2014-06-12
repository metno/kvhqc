
#ifndef COMMON_OBSPGMREQUEST_HH
#define COMMON_OBSPGMREQUEST_HH 1

#include "Sensor.hh"
#include "KvTypedefs.hh"

#include <QtCore/QObject> 

#include <map>

class QueryTaskHelper;
class SignalTask;

class ObsPgmRequest : public QObject
{ Q_OBJECT;
public:
  typedef std::map<int, hqc::kvObsPgm_v> kvObsPgm_m;

  ObsPgmRequest(const hqc::int_s& stationIds);
  ~ObsPgmRequest();

  void post();

  const hqc::kvObsPgm_v& operator[](int stationId) const
    { return get(stationId); }

  const hqc::kvObsPgm_v& get(int stationId) const;

Q_SIGNALS:
  void complete();

private Q_SLOTS:
  void onTaskDone(SignalTask*);

private:
  void put(const hqc::kvObsPgm_v& op);

private:
  kvObsPgm_m mObsPgms;
  static const hqc::kvObsPgm_v sEmpty;

  QueryTaskHelper *mTaskHelper;
};

#endif // COMMON_OBSPGMREQUEST_HH
