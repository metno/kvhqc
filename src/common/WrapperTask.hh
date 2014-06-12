
#ifndef COMMON_WRAPPERTASK_HH
#define COMMON_WRAPPERTASK_HH 1

#include "SignalTask.hh"

class WrapperTask : public SignalTask
{
public:
  WrapperTask(QueryTask* wrapped);
  ~WrapperTask();
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyStatus(int status);
  void notifyError(QString message);
  void notifyDone();
  
private:
  QueryTask* mWrapped;
};

#endif // COMMON_WRAPPERTASK_HH
