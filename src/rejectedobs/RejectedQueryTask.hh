
#ifndef COMMON_REJECTEDQUERYTASK_HH
#define COMMON_REJECTEDQUERYTASK_HH 1

#include "common/KvTypedefs.hh"
#include "common/QueryTask.hh"
#include "common/TimeSpan.hh"

class RejectedQueryTask : public QueryTask
{
public:
  RejectedQueryTask(const TimeSpan& time, size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyDone(const QString& withError);

  const hqc::kvRejectdecode_v& rejected() const
    { return mRejected; }
  
  hqc::kvRejectdecode_v& rejected()
    { return mRejected; }

private:
  TimeSpan mTime;

  hqc::kvRejectdecode_v mRejected;
};

#endif // COMMON_REJECTEDQUERYTASK_HH
