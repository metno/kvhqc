/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2014-2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "TasksColumn.hh"
#include "TaskData.hh"
#include "common/DataColumn.hh"
#include "common/Tasks.hh"
#include "util/stringutil.hh"
#include <QBrush>

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
    if (TaskData_p td = std::dynamic_pointer_cast<TaskData>(mDC->getObs(time))) {
      if (td->hasRequiredTasks())
        return QBrush(Qt::red);
      else if (td->hasTasks())
        return QBrush(QColor(0xFF, 0x60, 0)); // red orange
    }
  } else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
    QString tip;
    if (TaskData_p td = std::dynamic_pointer_cast<TaskData>(mDC->getObs(time)))
      tip = tasks::asText(td->allTasks());
    return Helpers::appendedText(tip, WrapperColumn::data(time, role).toString());
  }
  return WrapperColumn::data(time, role);
}
