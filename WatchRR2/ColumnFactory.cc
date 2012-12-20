
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
        c2t->addCode(-1, (QStringList()
                          << qApp->translate("Column_SA", "pat")
                          << qApp->translate("Column_SA", "p")),
                    qApp->translate("Column_SA", "patchy snow"));
        c2t->addCode(-3, (QStringList()
                          << qApp->translate("Column_SA", "no m.")
                          << qApp->translate("Column_SA", "n")),
                     qApp->translate("Column_SA", "measurement impossible/inaccurate"));
        c2t->setRange(0, 5000);
    } else if( pid == kvalobs::PARAMID_SD ) {
        c2t->addCode(-1, (QStringList()
                          << qApp->translate("Column_SD", "bare")
                          << qApp->translate("Column_SD", "b")),
                     qApp->translate("Column_SD", "snow cover not reported"));
        c2t->setRange(0, 5000);
    }
    return c2t;
}

DataColumnPtr columnForSensor(EditAccessPtr da, const Sensor& sensor, DataColumn::DisplayType displayType)
{
    const int pid = sensor.paramId;

    if( pid == kvalobs::PARAMID_V4 or pid == kvalobs::PARAMID_V5 or pid == kvalobs::PARAMID_V6 )
        return boost::make_shared<VxColumn>(da, sensor, displayType);
    DataColumnPtr dc;
    if( pid == kvalobs::PARAMID_RR )
        dc = boost::make_shared<RR24Column>(da, sensor, displayType);
    else
        dc = boost::make_shared<DataColumn>(da, sensor, displayType);
    dc->setCodes(codesForParam(pid));
    return dc;
}

ModelColumnPtr columnForSensor(ModelAccessPtr ma, const Sensor& sensor)
{
    const int pid = sensor.paramId;
    const Sensor ms = Helpers::modelSensor(sensor);

    ModelColumnPtr mc = boost::make_shared<ModelColumn>(ma, ms);
    mc->setCodes(codesForParam(pid));
    return mc;
}

} // namespace ColumnFactory
