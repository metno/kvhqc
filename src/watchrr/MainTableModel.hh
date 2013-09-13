
#ifndef WATCHRR_MAINTABLEMODEL_HH
#define WATCHRR_MAINTABLEMODEL_HH 1

#include "common/EditTimeColumn.hh"
#include "common/ObsColumn.hh"
#include "common/ObsTableModel.hh"
#include "common/ModelAccess.hh"

class MainTableModel : public ObsTableModel
{
public:
  MainTableModel(EditAccessPtr da, ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time);

  int getRR24Column() const;
  void setRR24TimeRange(const TimeRange& tr);

private:
  boost::shared_ptr<EditTimeColumn> mRR24EditTime;
};

#endif /* WATCHRR_MAINTABLEMODEL_HH */
