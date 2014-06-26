
#ifndef WATCHRR_NEIGHBORRR24MODEL_HH
#define WATCHRR_NEIGHBORRR24MODEL_HH 1

#include "TaskAccess.hh"
#include "WatchRRTableModel.hh"
#include <vector>

class NeighborRR24Model : public WatchRRTableModel
{
public:
  NeighborRR24Model(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time);
  virtual QVariant columnHeader(int section, Qt::Orientation orientation, int role) const;

private:
  std::vector<Sensor> mNeighbors;
};

#endif /* WATCHRR_NEIGHBORRR24MODEL_HH */
