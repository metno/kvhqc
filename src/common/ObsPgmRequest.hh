
#ifndef COMMON_OBSPGMREQUEST_HH
#define COMMON_OBSPGMREQUEST_HH 1

#include "Sensor.hh"
#include "KvTypedefs.hh"
#include "util/hqcObsPgm.h"

#include <QObject> 

#include <map>

class QueryTaskHelper;
class QueryTask;

class ObsPgmRequest : public QObject
{ Q_OBJECT;
public:
  typedef std::map<int, hqc::hqcObsPgm_v> hqcObsPgm_m;

  ObsPgmRequest(const hqc::int_s& stationIds);
  ObsPgmRequest(int stationId);
  ~ObsPgmRequest();

  void post();
  void sync();

  const hqc::hqcObsPgm_v& operator[](int stationId) const
    { return get(stationId); }

  const hqc::hqcObsPgm_v& get(int stationId) const;

Q_SIGNALS:
  void complete();

private Q_SLOTS:
  void onTaskDone(QueryTask*);

private:
  void init(const hqc::int_s& stationIds);
  void put(const hqc::hqcObsPgm_v& op);

private:
  hqcObsPgm_m mObsPgms;
  static const hqc::hqcObsPgm_v sEmpty;

  QueryTaskHelper *mTaskHelper;
};

#endif // COMMON_OBSPGMREQUEST_HH
