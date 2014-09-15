
#ifndef TASKSCOLUMN_HH
#define TASKSCOLUMN_HH 1

#include "WrapperColumn.hh"

class TasksColumn : public WrapperColumn {
public:
  TasksColumn(DataColumn_p dc);
  ~TasksColumn();

  virtual QVariant data(const timeutil::ptime& time, int role) const;
};

#endif // TASKSCOLUMN_HH
