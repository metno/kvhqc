
#ifndef COMMON_TEXTDATAQUERYTASK_HH
#define COMMON_TEXTDATAQUERYTASK_HH 1

#include "TxtDat.hh"

#include "common/QueryTask.hh"
#include "common/TimeSpan.hh"

class TextDataQueryTask : public QueryTask
{
public:
  TextDataQueryTask(const int stationId, const TimeSpan& time, size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyDone(const QString& withError);

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
