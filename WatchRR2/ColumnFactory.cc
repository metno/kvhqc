
#include "ColumnFactory.hh"

#include "DataControlinfoItem.hh"
#include "DataOriginalItem.hh"
#include "DataRR24Item.hh"
#include "DataVxItem.hh"
#include "HqcApplication.hh"
#include "KvMetaDataBuffer.hh"
#include "Sensor.hh"

#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.ColumnFactory"
#include "HqcLogging.hh"

namespace /* anonymous */ {

QString languageOrdering(QString languageColumn)
{
  METLIBS_LOG_SCOPE();
  QStringList uil = QLocale::system().uiLanguages();
  if (uil.isEmpty())
    uil << "C";

  QString lo = "(CASE " + languageColumn;
  for (int i=0; i<uil.size(); ++i)
    lo += QString(" WHEN '%1' THEN %2").arg(uil.at(i)).arg(i);
  lo += QString(" ELSE %1 END) ASC").arg(uil.size());
  METLIBS_LOG_DEBUG(LOGVAL(lo));
  return lo;
}

typedef std::map<int, Code2TextCPtr> Code2Text_t;
Code2Text_t sCode2Text; // FIXME these are leaked

} // namespace anonymous

namespace ColumnFactory {

Code2TextCPtr codesForParam(int pid)
{
    METLIBS_LOG_SCOPE();
    if (pid == kvalobs::PARAMID_V4 or pid == kvalobs::PARAMID_V5 or pid == kvalobs::PARAMID_V6
        or pid == kvalobs::PARAMID_V4S or pid == kvalobs::PARAMID_V5S or pid == kvalobs::PARAMID_V6S)
      return Code2TextPtr();

    const Code2Text_t::const_iterator c2t_it = sCode2Text.find(pid);
    if (c2t_it != sCode2Text.end())
      return c2t_it->second;

    Code2TextPtr c2t = boost::make_shared<Code2Text>();
    sCode2Text.insert(std::make_pair<int, Code2TextCPtr>(pid, c2t));

    int decimals = 1;
    try {
      const kvalobs::kvParam& param = KvMetaDataBuffer::instance()->findParam(pid);
      if (param.unit().find("kode") != std::string::npos) {
        decimals = 0;
      } else {
        QSqlQuery queryDecimals(hqcApp->systemDB());
        queryDecimals.prepare("SELECT decimals FROM param_decimals WHERE paramid = ?");
        queryDecimals.exec();
        if (queryDecimals.next())
          decimals = queryDecimals.value(0).toInt();
      }
    } catch (std::exception& ex) {
      METLIBS_LOG_WARN("exception while retrieving kvParam for " << pid);
    }
    c2t->setDecimals(decimals);

    const QString langOrder = languageOrdering("language");

    QSqlQuery queryCodes(hqcApp->systemDB());
    queryCodes.prepare("SELECT id, code_value FROM param_codes"
        " WHERE paramid = ? ORDER BY code_value ASC");

    QSqlQuery queryCodeLong(hqcApp->systemDB());
    queryCodeLong.prepare("SELECT long_text, language FROM param_code_long"
        " WHERE code_id = ? ORDER BY " + langOrder + " LIMIT 1");

    QSqlQuery queryCodeShort(hqcApp->systemDB());
    queryCodeShort.prepare("SELECT short_text FROM param_code_short"
        " WHERE code_id = ? AND language = ? ORDER BY sortkey ASC");

    queryCodes.bindValue(0, pid);
    if (queryCodes.exec()) {
      while (queryCodes.next()) {
        const int code_id = queryCodes.value(0).toInt();
        const int code_value = queryCodes.value(1).toInt();

        queryCodeLong.bindValue(0, code_id);
        if (queryCodeLong.exec()) {
          if (queryCodeLong.next()) {
            const QString long_text = queryCodeLong.value(0).toString();
            const QString language = queryCodeLong.value(1).toString();
            
            QStringList shortCodes;
            queryCodeShort.bindValue(0, code_id);
            queryCodeShort.bindValue(1, language);
            if (not queryCodeShort.exec())
              METLIBS_LOG_WARN("error getting short text for parameter " << pid << ", code " << code_value
                  << " and language '" << language << "' from system DB: " << queryCodeShort.lastError().text());
            while (queryCodeShort.next())
              shortCodes << queryCodeShort.value(0).toString();

            c2t->addCode(code_value, shortCodes, long_text);
          }
        } else {
          METLIBS_LOG_WARN("error getting long text text for parameter " << pid << ", code " << code_value
              << " from system DB: " << queryCodeLong.lastError().text());
        }
      }
    } else {
      METLIBS_LOG_ERROR("error getting code values for parameter " << pid
          << " from system DB: " << queryCodes.lastError().text());
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
    
    Code2TextCPtr codes = codesForParam(pid);
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
