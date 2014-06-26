
#ifndef TASKUPDATE_HH
#define TASKUPDATE_HH 1

#include "common/KvalobsUpdate.hh"

class TaskUpdate : public KvalobsUpdate {
public:
  TaskUpdate(const SensorTime& st);
  TaskUpdate(ObsData_p obs);

  int tasks() const
    { return mTasks; }

  void setTasks(int t)
    { mTasks = t; }
  
  void addTask(int id)
    { mTasks |= (1<<id); }
  
  void clearTask(int id)
    { mTasks &= ~(1<<id); }

  void clearTasks(int ids)
    { mTasks &= ~ids; }
  
private:
  int mTasks;
};

HQC_TYPEDEF_P(TaskUpdate);
HQC_TYPEDEF_PV(TaskUpdate);

#endif // TASKUPDATE_HH
