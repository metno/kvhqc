
#ifndef WATCHRR_NEIGHBORTABLEMODEL_HH
#define WATCHRR_NEIGHBORTABLEMODEL_HH 1

#include "common/ObsTableModel.hh"
#include <vector>

class NeighborTableModel : public ObsTableModel
{
public:
  NeighborTableModel(EditAccessPtr da, const Sensor& sensor, const TimeRange& time);
  virtual QVariant columnHeader(int section, Qt::Orientation orientation, int role) const;

private:
  std::vector<Sensor> mNeighbors;
};

#endif /* WATCHRR_NEIGHBORTABLEMODEL_HH */
