
#ifndef COMMON_PARAMQUERYTASK_HH
#define COMMON_PARAMQUERYTASK_HH 1

#include "SignalTask.hh"
#include "KvTypedefs.hh"

class ParamQueryTask : public SignalTask
{
public:
  ParamQueryTask(size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);

  const hqc::kvParam_v& params() const
    { return mParams; }

private:
  hqc::kvParam_v mParams;
};

#endif // COMMON_PARAMQUERYTASK_HH
