
#ifndef EDITTIMECOLUMN_HH
#define EDITTIMECOLUMN_HH 1

#include "TasksColumn.hh"
#include "common/TimeSpan.hh"

class EditTimeColumn : public TasksColumn {
public:
  EditTimeColumn(DataColumn_p dc);
  ~EditTimeColumn();
  
  virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
  virtual QVariant data(const timeutil::ptime& time, int role) const;
  virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);
  
  void setEditableTime(const TimeSpan& et);

private:
  TimeSpan mEditableTime;
};

#endif // EDITTIMECOLUMN_HH
