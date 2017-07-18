
#ifndef COMMON_ABSCTRACTUPDATELISTENER_HH
#define COMMON_ABSCTRACTUPDATELISTENER_HH 1

#include "KvTypedefs.hh"
#include <QObject>
#include <vector>

class AbstractUpdateListener : public QObject
{ Q_OBJECT;
public:
  virtual ~AbstractUpdateListener() { }

  virtual void addStation(int stationId) = 0;
  virtual void removeStation(int stationId) = 0;

Q_SIGNALS:
  void updated(const hqc::kvData_v& data);
  void update(const kvalobs::kvData& kvdata);
};

AbstractUpdateListener* updateListener();
void setUpdateListener(AbstractUpdateListener* ul);

#endif // COMMON_ABSCTRACTUPDATELISTENER_HH
