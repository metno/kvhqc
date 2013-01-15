
#ifndef ColumnFactory_hh
#define ColumnFactory_hh 1

#include "DataColumn.hh"
#include "DataItem.hh"
#include "ModelColumn.hh"
class Sensor;
class EditAccess;
typedef boost::shared_ptr<EditAccess> EditAccessPtr;

namespace ColumnFactory {

enum DisplayType { ORIGINAL,
                   OLD_CORRECTED, NEW_CORRECTED,
                   OLD_CONTROLINFO, NEW_CONTROLINFO,
                   N_DISPLAYTYPES };

Code2TextPtr codesForParam(int paramId);

DataItemPtr itemForSensor(EditAccessPtr da, const Sensor& sensor, DisplayType displayType);

DataColumnPtr columnForSensor(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, DisplayType displayType);

ModelColumnPtr columnForSensor(ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time);

} // namespace ColumnFactory

#endif // ColumnFactory_hh
