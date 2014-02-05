
#ifndef COMMON_ABSCTRACTUPDATELISTENER_HH
#define COMMON_ABSCTRACTUPDATELISTENER_HH 1

#include <QtCore/QObject>

namespace kvalobs {
class kvData;
}

class AbstractUpdateListener : public QObject
{ Q_OBJECT;
public:
  virtual ~AbstractUpdateListener() { }

  virtual void addStation(int stationId) = 0;
  virtual void removeStation(int stationId) = 0;
  
Q_SIGNALS:
  void update(const kvalobs::kvData& kvdata);
};

AbstractUpdateListener* updateListener();
void setUpdateListener(AbstractUpdateListener* ul);

#endif // COMMON_ABSCTRACTUPDATELISTENER_HH
