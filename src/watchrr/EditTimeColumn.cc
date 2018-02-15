
#include "EditTimeColumn.hh"

#include <QBrush>

#define MILOGGER_CATEGORY "kvhqc.EditTimeColumn"
#include "util/HqcLogging.hh"

EditTimeColumn::EditTimeColumn(DataColumn_p dc)
  : TasksColumn(dc)
{
}

EditTimeColumn::~EditTimeColumn()
{
}

Qt::ItemFlags EditTimeColumn::flags(const timeutil::ptime& time) const
{
  METLIBS_LOG_SCOPE();
  Qt::ItemFlags flags = TasksColumn::flags(time);
  const Qt::ItemFlags editFlags = Qt::ItemIsSelectable | Qt::ItemIsEditable;
  if (mEditableTime.contains(time))
    flags |= editFlags;
  else
    flags &= ~editFlags;
  return flags;
}

QVariant EditTimeColumn::data(const timeutil::ptime& time, int role) const
{
  if (role == Qt::BackgroundRole and not mEditableTime.contains(time))
    return QBrush(Qt::lightGray);
  return TasksColumn::data(time, role);
}

bool EditTimeColumn::setData(const timeutil::ptime& time, const QVariant& value, int role)
{
  if (not mEditableTime.contains(time))
    return false;
  return TasksColumn::setData(time, value, role);
}

void EditTimeColumn::setEditableTime(const TimeSpan& et)
{
  mEditableTime = et;
}
