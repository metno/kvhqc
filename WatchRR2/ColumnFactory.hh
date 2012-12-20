
#ifndef ColumnFactory_hh
#define ColumnFactory_hh 1

#include "DataColumn.hh"
#include "ModelColumn.hh"
class Sensor;
class EditAccess;
typedef boost::shared_ptr<EditAccess> EditAccessPtr;

namespace ColumnFactory {

Code2TextPtr codesForParam(int paramId);

DataColumnPtr columnForSensor(EditAccessPtr da, const Sensor& sensor, DataColumn::DisplayType displayType);

ModelColumnPtr columnForSensor(ModelAccessPtr ma, const Sensor& sensor);

} // namespace ColumnFactory

#endif // ColumnFactory_hh
