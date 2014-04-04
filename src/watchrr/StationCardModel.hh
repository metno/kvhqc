
#ifndef WATCHRR_STATIONCARDMODEL_HH
#define WATCHRR_STATIONCARDMODEL_HH 1

#include "common/EditTimeColumn.hh"
#include "common/ObsColumn.hh"
#include "common/ObsTableModel.hh"
#include "common/ModelAccess.hh"

class StationCardModel : public ObsTableModel
{
public:
  StationCardModel(EditAccessPtr da, ModelAccessPtr ma, const Sensor& sensor, const TimeSpan& time);

  int getRR24Column() const;
  void setRR24TimeSpan(const TimeSpan& tr);

private:
  boost::shared_ptr<EditTimeColumn> mRR24EditTime;
};

#endif /* WATCHRR_STATIONCARDMODEL_HH */
