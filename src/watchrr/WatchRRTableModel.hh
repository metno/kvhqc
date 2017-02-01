
#ifndef WATCHRRTABLEMODEL_HH
#define WATCHRRTABLEMODEL_HH 1

#include "common/ObsTableModel.hh"
#include "TaskAccess.hh"

class WatchRRTableModel : public ObsTableModel
{
public:
  WatchRRTableModel(TaskAccess_p da, QObject* parent=0)
    : ObsTableModel(da, 24*60*60, parent) { }

protected:
  TaskAccess_p ta() const
    { return std::static_pointer_cast<TaskAccess>(mDA); }
};

#endif /* WATCHRRTABLEMODEL_HH */
