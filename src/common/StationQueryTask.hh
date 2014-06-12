
#ifndef COMMON_STATIONQUERYTASK_HH
#define COMMON_STATIONQUERYTASK_HH 1

#include "SignalTask.hh"
#include "KvTypedefs.hh"

class StationQueryTask : public SignalTask
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
