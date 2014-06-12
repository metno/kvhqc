
#ifndef COMMON_TYPESQUERYTASK_HH
#define COMMON_TYPESQUERYTASK_HH 1

#include "SignalTask.hh"
#include "KvTypedefs.hh"

class TypesQueryTask : public SignalTask
{
public:

  TypesQueryTask(size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);

  const hqc::kvTypes_v& types() const
    { return mTypes; }

private:
  hqc::kvTypes_v mTypes;
};

#endif // COMMON_TYPESQUERYTASK_HH
