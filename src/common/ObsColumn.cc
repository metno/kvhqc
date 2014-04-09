
#include "ObsColumn.hh"

void ObsColumn::attach(ObsTableModel*)
{
}

void ObsColumn::detach(ObsTableModel*)
{
}

Qt::ItemFlags ObsColumn::flags(const timeutil::ptime& /*time*/) const
{
    return Qt::ItemIsEnabled;
}

bool ObsColumn::setData(const timeutil::ptime& /*time*/, const QVariant& /*value*/, int /*role*/)
{
    return false;
}

const Sensor& ObsColumn::sensor() const
{
  static const Sensor invalid;
  return invalid;
}
