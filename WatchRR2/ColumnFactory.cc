
#include "ColumnFactory.hh"

#include "DataControlinfoItem.hh"
#include "DataOriginalItem.hh"
#include "DataRR24Item.hh"
#include "DataVxItem.hh"
#include "HqcApplication.hh"
#include "KvMetaDataBuffer.hh"
#include "Sensor.hh"

#include <QtCore/QCoreApplication>
#include <QtSql/QSqlQuery>
#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.ColumnFactory"
#include "HqcLogging.hh"

namespace ColumnFactory {

Code2TextPtr codesForParam(int pid)
{
    METLIBS_LOG_SCOPE();
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
    } else {
      try {
        const kvalobs::kvParam& param = KvMetaDataBuffer::instance()->findParam(pid);
        if (param.unit().find("kode") != std::string::npos) {
          c2t->setDecimals(0);

          QSqlQuery query(hqcApp->systemDB());
          query.prepare("SELECT code, description FROM code_explain WHERE paramid = ? AND language = 'en' ORDER BY code ASC");
          query.bindValue(0, pid);
          query.exec();
          while (query.next()) {
            const int code = query.value(0).toInt();
            const QString desc = query.value(1).toString();
            c2t->addCode(code, QStringList(), desc);
          }
        }
      } catch (std::exception& ex) {
        METLIBS_LOG_WARN("exception while retrieving kvParam for " << pid);
      }
    }
    return c2t;
}

DataItemPtr itemForSensor(EditAccessPtr da, const Sensor& sensor, ObsColumn::Type displayType)
{
    const int pid = sensor.paramId;

    DataItemPtr item;
    if ((pid == kvalobs::PARAMID_V4 or pid == kvalobs::PARAMID_V5 or pid == kvalobs::PARAMID_V6)
        and (displayType == ObsColumn::NEW_CORRECTED or displayType == ObsColumn::OLD_CORRECTED or displayType == ObsColumn::ORIGINAL))
    {
      return boost::make_shared<DataVxItem>(displayType, da);
    }
    if (displayType == ObsColumn::OLD_CONTROLINFO or displayType == ObsColumn::NEW_CONTROLINFO) {
      const bool showNew = displayType == ObsColumn::NEW_CONTROLINFO;
      return boost::make_shared<DataControlinfoItem>(showNew);
    }
    
    Code2TextPtr codes = codesForParam(pid);
    if (displayType == ObsColumn::OLD_CORRECTED or displayType == ObsColumn::NEW_CORRECTED) {
      const bool showNew = displayType == ObsColumn::NEW_CORRECTED;
      if (pid == kvalobs::PARAMID_RR_24)
        return boost::make_shared<DataRR24Item>(showNew, codes);
      else
        return boost::make_shared<DataCorrectedItem>(showNew, codes);
    } else if (displayType == ObsColumn::ORIGINAL) {
      return boost::make_shared<DataOriginalItem>(codes);
    }
    return DataItemPtr();
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
