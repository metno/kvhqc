
#ifndef COMMON_TEXTDATAQUERYTASK_HH
#define COMMON_TEXTDATAQUERYTASK_HH 1

#include "TxtDat.hh"

#include "common/SignalTask.hh"
#include "common/TimeSpan.hh"

class TextDataQueryTask : public SignalTask
{
public:
  TextDataQueryTask(const int stationId, const TimeSpan& time, size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyStatus(int status);

  const TxtDat_v& textData() const
    { return mTextData; }

  TxtDat_v& textData()
    { return mTextData; }

private:
  int mStationId;
  TimeSpan mTime;

  TxtDat_v mTextData;
};

#endif // COMMON_TEXTDATAQUERYTASK_HH
