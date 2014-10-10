
#ifndef COMMON_OBSPGMQUERYTASK_HH
#define COMMON_OBSPGMQUERYTASK_HH 1

#include "QueryTask.hh"
#include <kvalobs/kvObsPgm.h>
#include <QtCore/QObject>
#include <vector>
#include <set>

class ObsPgmQueryTask : public QueryTask
{ Q_OBJECT;
public:
  typedef std::vector<kvalobs::kvObsPgm> kvObsPgm_v;
  typedef std::set<int> int_s;

  ObsPgmQueryTask(const int_s& stationIds, size_t priority);
  ~ObsPgmQueryTask();
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);

  const kvObsPgm_v& obsPgms() const
    { return mObsPgms; }

private:
  int_s mStationIds;
  kvObsPgm_v mObsPgms;
};

#endif // COMMON_OBSPGMQUERYTASK_HH
