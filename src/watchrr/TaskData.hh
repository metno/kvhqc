
#ifndef TASKDATA_HH
#define TASKDATA_HH 1

#include "common/KvalobsData.hh"

class TaskData : public KvalobsData
{
public:
  TaskData(KvalobsData_p kd)
    : KvalobsData(kd->data(), kd->isCreated()), mTasks(0) { }

  bool hasTasks() const
    { return allTasks() != 0; }

  bool hasRequiredTasks() const;

  bool hasTask(int id) const
    { return (allTasks() & (1<<id)) != 0; }

  bool modifiedTasks() const
    { return mTasks != 0; }

  int allTasks() const
    { return mTasks; }

private:
  int mTasks;
};

HQC_TYPEDEF_P(TaskData);

#endif // TASKDATA_HH
