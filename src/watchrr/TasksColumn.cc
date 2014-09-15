
#include "TasksColumn.hh"
#include "TaskData.hh"
#include "common/Tasks.hh"
#include "util/Helpers.hh"
#include <QtGui/QBrush>

#define MILOGGER_CATEGORY "kvhqc.TasksColumn"
#include "util/HqcLogging.hh"

TasksColumn::TasksColumn(DataColumn_p dc)
  : WrapperColumn(dc)
{
}

TasksColumn::~TasksColumn()
{
}

QVariant TasksColumn::data(const timeutil::ptime& time, int role) const
{
  if (role == Qt::BackgroundRole) {
    if (TaskData_p td = boost::dynamic_pointer_cast<TaskData>(mDC->getObs(time))) {
      if (td->hasRequiredTasks())
        return QBrush(Qt::red);
      else if (td->hasTasks())
        return QBrush(QColor(0xFF, 0x60, 0)); // red orange
    }
  } else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
    QString tip;
    if (TaskData_p td = boost::dynamic_pointer_cast<TaskData>(mDC->getObs(time)))
      tip = tasks::asText(td->allTasks());
    return Helpers::appendedText(tip, WrapperColumn::data(time, role).toString());
  }
  return WrapperColumn::data(time, role);
}
