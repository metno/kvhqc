
#include "ColumnFactory.hh"

#include "DataControlinfoItem.hh"
#include "DataOriginalItem.hh"
#include "DataRR24Item.hh"
#include "DataVxItem.hh"
#include "Sensor.hh"

#include <QtCore/QCoreApplication>
#include <boost/make_shared.hpp>

namespace ColumnFactory {

Code2TextPtr codesForParam(int pid)
{
    if (pid == kvalobs::PARAMID_V4 or pid == kvalobs::PARAMID_V5 or pid == kvalobs::PARAMID_V6)
        return Code2TextPtr();

    Code2TextPtr c2t = boost::make_shared<Code2Text>();
    if( pid == kvalobs::PARAMID_RR_24 ) {
        c2t->addCode(-1, (QStringList()
                          << qApp->translate("Column_RR_24", "dry")
                          << qApp->translate("Column_RR_24", "d")),
                     qApp->translate("Column_RR_24", "precipitation not reported"));
        c2t->setRange(0, 1500);
    } else if( pid == kvalobs::PARAMID_SA ) {
#if 0 // removed on request by POK on 2013-01-14
        c2t->addCode(-1, (QStringList()
                          << qApp->translate("Column_SA", "pat")
                          << qApp->translate("Column_SA", "p")),
                     qApp->translate("Column_SA", "patchy snow"));
#endif
        c2t->addCode(-3, (QStringList()
                          << qApp->translate("Column_SA", "no m.")
                          << qApp->translate("Column_SA", "n")),
                     qApp->translate("Column_SA", "measurement impossible/inaccurate"));
        c2t->setRange(-1, 5000);
        c2t->setDecimals(0);
    } else if( pid == kvalobs::PARAMID_SD ) {
        c2t->addCode(-1, (QStringList()
                          << qApp->translate("Column_SD", "no m.")
                          << qApp->translate("Column_SD", "n")),
                     qApp->translate("Column_SD", "snow cover not reported"));
        c2t->setRange(-1, 4);
        c2t->setDecimals(0);
    }
    return c2t;
}

DataItemPtr itemForSensor(EditAccessPtr da, const Sensor& sensor, ObsColumn::Type displayType)
{
    const int pid = sensor.paramId;

    DataItemPtr item;
    if (pid == kvalobs::PARAMID_V4 or pid == kvalobs::PARAMID_V5 or pid == kvalobs::PARAMID_V6) {
      if (displayType == ObsColumn::NEW_CORRECTED)
        item = boost::make_shared<DataVxItem>(da);
    } else if (displayType == ObsColumn::OLD_CONTROLINFO or displayType == ObsColumn::NEW_CONTROLINFO) {
      const bool showNew = displayType == ObsColumn::NEW_CONTROLINFO;
        item = boost::make_shared<DataControlinfoItem>(showNew);
    } else {
        Code2TextPtr codes = codesForParam(pid);
        if (displayType == ObsColumn::OLD_CORRECTED or displayType == ObsColumn::NEW_CORRECTED) {
            const bool showNew = displayType == ObsColumn::NEW_CORRECTED;
            if (pid == kvalobs::PARAMID_RR_24)
                item = boost::make_shared<DataRR24Item>(showNew, codes);
            else
                item = boost::make_shared<DataCorrectedItem>(showNew, codes);
        } else if (displayType == ObsColumn::ORIGINAL) {
            item = boost::make_shared<DataOriginalItem>(codes);
        }
    }
    return item;
}

DataColumnPtr columnForSensor(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, ObsColumn::Type displayType)
{
  DataItemPtr item = itemForSensor(da, sensor, displayType);
  if (item)
    return boost::make_shared<DataColumn>(da, sensor, time, item);
  return DataColumnPtr();
}

ModelColumnPtr columnForSensor(ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time)
{
    const int pid = sensor.paramId;
    const Sensor ms = ModelAccess::makeModelSensor(sensor);

    ModelColumnPtr mc = boost::make_shared<ModelColumn>(ma, ms, time);
    mc->setCodes(codesForParam(pid));
    return mc;
}

} // namespace ColumnFactory
