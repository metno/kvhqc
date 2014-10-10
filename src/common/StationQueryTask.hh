
#ifndef COMMON_STATIONQUERYTASK_HH
#define COMMON_STATIONQUERYTASK_HH 1

#include "QueryTask.hh"
#include "KvTypedefs.hh"

class StationQueryTask : public QueryTask
{
public:
  StationQueryTask(size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);

  const hqc::kvStation_v& stations() const
    { return mStations; }

private:
  hqc::kvStation_v mStations;
};

#endif // COMMON_STATIONQUERYTASK_HH
