
#ifndef COMMON_REJECTEDQUERYTASK_HH
#define COMMON_REJECTEDQUERYTASK_HH 1

#include "common/KvTypedefs.hh"
#include "common/SignalTask.hh"
#include "common/TimeSpan.hh"

class RejectedQueryTask : public SignalTask
{
public:
  RejectedQueryTask(const TimeSpan& time, size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyStatus(int status);

  const hqc::kvRejectdecode_v& rejected() const
    { return mRejected; }
  
  hqc::kvRejectdecode_v& rejected()
    { return mRejected; }

private:
  TimeSpan mTime;

  hqc::kvRejectdecode_v mRejected;
};

#endif // COMMON_REJECTEDQUERYTASK_HH
