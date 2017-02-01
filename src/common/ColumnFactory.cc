
#include "ColumnFactory.hh"

#include "DataControlinfoItem.hh"
#include "DataOriginalItem.hh"
#include "DataRR24Item.hh"
#include "DataVxItem.hh"
#include "KvMetaDataBuffer.hh"
#include "Sensor.hh"
#include "common/HqcSystemDB.hh"
#include "common/KvHelpers.hh"

#define MILOGGER_CATEGORY "kvhqc.ColumnFactory"
#include "util/HqcLogging.hh"

namespace /* anonymous */ {

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

    Code2TextPtr c2t = std::make_shared<Code2Text>();
    sCode2Text.insert(std::make_pair(pid, c2t));

    bool haveDecimals = false;
    int decimals = 1;
    if (KvMetaDataBuffer* kvmdb = KvMetaDataBuffer::instance()) {
      try {
        if (kvmdb->isCodeParam(pid) or kvmdb->isDirectionInDegreesParam(pid)) {
          haveDecimals = true;
          decimals = 0;
        }
      } catch (std::exception& ex) {
        HQC_LOG_WARN("exception while retrieving kvParam for " << pid);
      }
    }
    if (not haveDecimals) {
      if (HqcSystemDB::shownDecimals(pid, decimals))
        haveDecimals = true;
    }
    if (haveDecimals)
      c2t->setDecimals(decimals);

    const HqcSystemDB::ParamCode_ql pcl = HqcSystemDB::paramCodes(pid);
    for (HqcSystemDB::ParamCode_ql::const_iterator it = pcl.begin(); it != pcl.end(); ++it)
      c2t->addCode(it->value, it->shortTexts, it->longText);

    return c2t;
}

DataItem_p itemForSensor(EditAccess_p da, const Sensor& sensor, ObsColumn::Type displayType)
{
    const int pid = sensor.paramId;

    DataItem_p item;
    if ((pid == kvalobs::PARAMID_V4 or pid == kvalobs::PARAMID_V5 or pid == kvalobs::PARAMID_V6)
        and (displayType == ObsColumn::NEW_CORRECTED or displayType == ObsColumn::ORIGINAL))
    {
      return std::make_shared<DataVxItem>(displayType, da);
    }
    if (displayType == ObsColumn::NEW_CONTROLINFO) {
      return std::make_shared<DataControlinfoItem>();
    }
    
    Code2TextCPtr codes = codesForParam(pid);
    if (displayType == ObsColumn::NEW_CORRECTED) {
      if (pid == kvalobs::PARAMID_RR_24)
        return std::make_shared<DataRR24Item>(codes);
      else
        return std::make_shared<DataCorrectedItem>(codes);
    } else if (displayType == ObsColumn::ORIGINAL) {
      return std::make_shared<DataOriginalItem>(codes);
    }
    return DataItem_p();
}

DataColumn_p columnForSensor(EditAccess_p da, const Sensor& sensor, const TimeSpan& time, ObsColumn::Type displayType)
{
  DataItem_p item = itemForSensor(da, sensor, displayType);
  if (item)
    return std::make_shared<DataColumn>(da, sensor, time, item);
  return DataColumn_p();
}

ModelColumn_p columnForSensor(ModelAccess_p ma, const Sensor& sensor, const TimeSpan& time)
{
  ModelColumn_p mc = std::make_shared<ModelColumn>(ma, sensor, time);
  mc->setCodes(codesForParam(sensor.paramId));
  return mc;
}

} // namespace ColumnFactory
