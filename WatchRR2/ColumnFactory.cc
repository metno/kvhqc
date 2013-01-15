
#include "ColumnFactory.hh"

#include "RR24Column.hh"
#include "Sensor.hh"
#include "VxColumn.hh"

#include <QtCore/QCoreApplication>
#include <boost/make_shared.hpp>

namespace ColumnFactory {

Code2TextPtr codesForParam(int pid)
{
    if( pid == kvalobs::PARAMID_V4 or pid == kvalobs::PARAMID_V5 or pid == kvalobs::PARAMID_V6 )
        return Code2TextPtr();

    Code2TextPtr c2t = boost::make_shared<Code2Text>();
    if( pid == kvalobs::PARAMID_RR ) {
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
        c2t->setRange(0, 5000);
        c2t->setDecimals(0);
    } else if( pid == kvalobs::PARAMID_SD ) {
        c2t->addCode(-1, (QStringList()
                          << qApp->translate("Column_SD", "bare")
                          << qApp->translate("Column_SD", "b")),
                     qApp->translate("Column_SD", "snow cover not reported"));
        c2t->setRange(0, 5000);
    }
    return c2t;
}

DataColumnPtr columnForSensor(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, DataColumn::DisplayType displayType)
{
    const int pid = sensor.paramId;

    if( pid == kvalobs::PARAMID_V4 or pid == kvalobs::PARAMID_V5 or pid == kvalobs::PARAMID_V6 )
        return boost::make_shared<VxColumn>(da, sensor, time, displayType);
    DataColumnPtr dc;
    if( pid == kvalobs::PARAMID_RR )
        dc = boost::make_shared<RR24Column>(da, sensor, time, displayType);
    else
        dc = boost::make_shared<DataColumn>(da, sensor, time, displayType);
    dc->setCodes(codesForParam(pid));
    return dc;
}

ModelColumnPtr columnForSensor(ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time)
{
    const int pid = sensor.paramId;
    const Sensor ms = Helpers::modelSensor(sensor);

    ModelColumnPtr mc = boost::make_shared<ModelColumn>(ma, ms, time);
    mc->setCodes(codesForParam(pid));
    return mc;
}

} // namespace ColumnFactory
