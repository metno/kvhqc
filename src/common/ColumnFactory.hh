
#ifndef ColumnFactory_hh
#define ColumnFactory_hh 1

#include "DataColumn.hh"
#include "DataItem.hh"
#include "ModelColumn.hh"
class Sensor;
class EditAccess;
typedef boost::shared_ptr<EditAccess> EditAccess_p;

namespace ColumnFactory {

Code2TextCPtr codesForParam(int paramId);

DataItem_p itemForSensor(EditAccess_p da, const Sensor& sensor, ObsColumn::Type displayType);

DataColumn_p columnForSensor(EditAccess_p da, const Sensor& sensor, const TimeRange& time, ObsColumn::Type displayType);

ModelColumnPtr columnForSensor(ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time);

} // namespace ColumnFactory

#endif // ColumnFactory_hh
