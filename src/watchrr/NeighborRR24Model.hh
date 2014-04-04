
#ifndef WATCHRR_NEIGHBORRR24MODEL_HH
#define WATCHRR_NEIGHBORRR24MODEL_HH 1

#include "common/ObsTableModel.hh"
#include <vector>

class NeighborRR24Model : public ObsTableModel
{
public:
  NeighborRR24Model(EditAccessPtr da, const Sensor& sensor, const TimeSpan& time);
  virtual QVariant columnHeader(int section, Qt::Orientation orientation, int role) const;

private:
  std::vector<Sensor> mNeighbors;
};

#endif /* WATCHRR_NEIGHBORRR24MODEL_HH */
