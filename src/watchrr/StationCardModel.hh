
#ifndef WATCHRR_STATIONCARDMODEL_HH
#define WATCHRR_STATIONCARDMODEL_HH 1

#include "EditTimeColumn.hh"
#include "WatchRRTableModel.hh"
#include "common/ObsColumn.hh"
#include "common/ModelAccess.hh"

class StationCardModel : public WatchRRTableModel
{
public:
  StationCardModel(TaskAccess_p da, ModelAccess_p ma, const Sensor& sensor, const TimeSpan& time);

  int getRR24CorrectedColumn() const;
  int getRR24OriginalColumn() const;
  void setRR24TimeSpan(const TimeSpan& tr);

private:
  std::shared_ptr<EditTimeColumn> mRR24EditTime;
};

#endif /* WATCHRR_STATIONCARDMODEL_HH */
