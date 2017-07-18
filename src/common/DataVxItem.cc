
#include "DataVxItem.hh"

#include "FlagChange.hh"
#include "ObsHelpers.hh"
#include "KvHelpers.hh"
#include "ObsColumn.hh"
#include "Tasks.hh"
#include "util/stringutil.hh"

#include <QStringList>
#include <QVariant>
#include <QApplication>
#include <QFont>

#define MILOGGER_CATEGORY "kvhqc.DataVxItem"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {
struct VxData {
    int code;
    const char* metCode;
    const char* explain;
};

// see http://metklim.met.no/nasjonale_koder
const VxData vxdata_312[] = {
    {  0, "",   QT_TRANSLATE_NOOP("DataVxItem", "no data") },
    {  3, "RR", QT_TRANSLATE_NOOP("DataVxItem", "rain") },
    {  7, "RB", QT_TRANSLATE_NOOP("DataVxItem", "rain shower") },
    {  2, "SS", QT_TRANSLATE_NOOP("DataVxItem", "snow") },
    {  5, "SB", QT_TRANSLATE_NOOP("DataVxItem", "snow shower") },
    {  1, "SL", QT_TRANSLATE_NOOP("DataVxItem", "sleet") },
    {  4, "LB", QT_TRANSLATE_NOOP("DataVxItem", "sleet shower") },
    {  8, "YR", QT_TRANSLATE_NOOP("DataVxItem", "drizzle") },
    { 10, "HG", QT_TRANSLATE_NOOP("DataVxItem", "hail") },
    { 12, "DU", QT_TRANSLATE_NOOP("DataVxItem", "dew") },
    { 17, "RI", QT_TRANSLATE_NOOP("DataVxItem", "rime") },
    { 20, "TO", QT_TRANSLATE_NOOP("DataVxItem", "thunder") },
    { 28, "SF", QT_TRANSLATE_NOOP("DataVxItem", "snowflakes") },
    {  6, "KS", QT_TRANSLATE_NOOP("DataVxItem", "Kornsnoe") },
    {  9, "SH", QT_TRANSLATE_NOOP("DataVxItem", "Sproehagl") },
    { 11, "IH", QT_TRANSLATE_NOOP("DataVxItem", "ice hail") },
    { 13, "TR", QT_TRANSLATE_NOOP("DataVxItem", "fog rime") },
    { 14, "IS", QT_TRANSLATE_NOOP("DataVxItem", "Isslag") },
    { 15, "IK", QT_TRANSLATE_NOOP("DataVxItem", "ice grains") },
    { 16, "IN", QT_TRANSLATE_NOOP("DataVxItem", "ice needles") },		
    { 18, "TA", QT_TRANSLATE_NOOP("DataVxItem", "fog") },
    { 19, "TD", QT_TRANSLATE_NOOP("DataVxItem", "fog mist") },
    { 21, "OO", QT_TRANSLATE_NOOP("DataVxItem", "oil smoke") },		
    { 22, "RL", QT_TRANSLATE_NOOP("DataVxItem", "clear air") },
    { 23, "HA", QT_TRANSLATE_NOOP("DataVxItem", "halo") },
    { 24, "KR", QT_TRANSLATE_NOOP("DataVxItem", "Krans") },
    { 25, "SO", QT_TRANSLATE_NOOP("DataVxItem", "sun") },
    { 26, "NL", QT_TRANSLATE_NOOP("DataVxItem", "northern light") },
    { 27, "RE", QT_TRANSLATE_NOOP("DataVxItem", "rainbow") },
    { 29, "KM", QT_TRANSLATE_NOOP("DataVxItem", "sheet lightning") },
    { 30, "RS", QT_TRANSLATE_NOOP("DataVxItem", "rain, snow, sleet") },
    { 31, "AN", QT_TRANSLATE_NOOP("DataVxItem", "dew, rime, fog") },
    { -1, 0, 0 }
};

const VxData vxdata_302_before2006[] = {
    {  0, "",   QT_TRANSLATE_NOOP("DataVxItem", "no data") },
    { 10, "RR", QT_TRANSLATE_NOOP("DataVxItem", "rain") },
    { 14, "RB", QT_TRANSLATE_NOOP("DataVxItem", "rain shower") },
    { 11, "SS", QT_TRANSLATE_NOOP("DataVxItem", "snow") },
    { 15, "SB", QT_TRANSLATE_NOOP("DataVxItem", "snow shower") },
    { 12, "SL", QT_TRANSLATE_NOOP("DataVxItem", "sleet") },
    { 16, "LB", QT_TRANSLATE_NOOP("DataVxItem", "sleet shower") },
    { 13, "YR", QT_TRANSLATE_NOOP("DataVxItem", "drizzle") },
    { 17, "HG", QT_TRANSLATE_NOOP("DataVxItem", "hail") },
    { 18, "DU", QT_TRANSLATE_NOOP("DataVxItem", "dew") },
    { 19, "RI", QT_TRANSLATE_NOOP("DataVxItem", "rime") },
    { 20, "TO", QT_TRANSLATE_NOOP("DataVxItem", "thunder") },
    { 22, "SF", QT_TRANSLATE_NOOP("DataVxItem", "snowflakes") },
    { -1, 0, 0 }
};

const VxData vxdata_302_after2006[] = {
    {  0, "",   QT_TRANSLATE_NOOP("DataVxItem", "no data") },

    { 10, "RR", QT_TRANSLATE_NOOP("DataVxItem", "rain weak") },
    { 11, "RR", QT_TRANSLATE_NOOP("DataVxItem", "rain") },
    { 12, "RR", QT_TRANSLATE_NOOP("DataVxItem", "rain strong") },

    { 40, "RB", QT_TRANSLATE_NOOP("DataVxItem", "rain shower weak") },
    { 41, "RB", QT_TRANSLATE_NOOP("DataVxItem", "rain shower") },
    { 42, "RB", QT_TRANSLATE_NOOP("DataVxItem", "rain shower strong") },

    { 20, "SS", QT_TRANSLATE_NOOP("DataVxItem", "snow weak") },
    { 21, "SS", QT_TRANSLATE_NOOP("DataVxItem", "snow") },
    { 22, "SS", QT_TRANSLATE_NOOP("DataVxItem", "snow strong") },

    { 50, "SB", QT_TRANSLATE_NOOP("DataVxItem", "snow shower weak") },
    { 51, "SB", QT_TRANSLATE_NOOP("DataVxItem", "snow shower") },
    { 52, "SB", QT_TRANSLATE_NOOP("DataVxItem", "snow shower strong") },

    { 30, "SL", QT_TRANSLATE_NOOP("DataVxItem", "sleet weak") },
    { 31, "SL", QT_TRANSLATE_NOOP("DataVxItem", "sleet") },
    { 32, "SL", QT_TRANSLATE_NOOP("DataVxItem", "sleet strong") },

    { 60, "LB", QT_TRANSLATE_NOOP("DataVxItem", "sleet shower weak") },
    { 61, "LB", QT_TRANSLATE_NOOP("DataVxItem", "sleet shower") },
    { 62, "LB", QT_TRANSLATE_NOOP("DataVxItem", "sleet shower strong") },

    { 73, "YR", QT_TRANSLATE_NOOP("DataVxItem", "drizzle") },
    { 74, "HG", QT_TRANSLATE_NOOP("DataVxItem", "hail") },
    { 85, "DU", QT_TRANSLATE_NOOP("DataVxItem", "dew") },
    { 86, "RI", QT_TRANSLATE_NOOP("DataVxItem", "rime") },
    { 97, "TO", QT_TRANSLATE_NOOP("DataVxItem", "thunder") },
    { 98, "SF", QT_TRANSLATE_NOOP("DataVxItem", "snowflakes") },
    { -1, 0, 0 }
};

const timeutil::ptime LIMIT_302(boost::gregorian::date(2006, 1, 9), boost::posix_time::time_duration(0, 0, 0));

const VxData* vxData4SensorTime(const SensorTime& st)
{
#if 1
  return vxdata_312;
#else
  if (st.sensor.typeId == 312)
    return vxdata_312;
  else if (st.sensor.typeId != 302)
    return 0;
  if (st.time >= LIMIT_302)
    return vxdata_302_after2006;
  else
    return vxdata_302_before2006;
#endif
}

} // namespace anonymous

DataVxItem::DataVxItem(ObsColumn::Type columnType, EditAccess_p da)
  : DataValueItem(columnType)
  , mDA(da)
{
}

QVariant DataVxItem::data(ObsData_p obs1, const SensorTime& st, int role) const
{
  const VxData* vxdata = vxData4SensorTime(st);
  if (not vxdata)
    HQC_LOG_WARN("no Vx codes known for " << st);

  if (role == ObsColumn::ValueTypeRole) {
    return ObsColumn::TextCode;
  } else if (role == ObsColumn::TextCodesRole or role == ObsColumn::TextCodeExplanationsRole) {
    QStringList codes;
    for(int i=0; vxdata and vxdata[i].code >= 0; ++i) {
      if (role == ObsColumn::TextCodesRole) {
        QString mc = vxdata[i].metCode;
        if (vxdata[i].code != 0) {
          codes << (mc + QChar( 0xB0 ))
                << mc
                << (mc + QChar( 0xB2 ));
        } else {
          codes << "";
        }
      } else {
        QString tooltip = qApp->translate("DataVxItem", vxdata[i].explain);
        if (vxdata[i].code != 0) {
          codes << (tooltip + " -- " + qApp->translate("DataVxItem", "weak"))
                << tooltip
                << (tooltip + " -- " + qApp->translate("DataVxItem", "strong"));
        } else {
          codes << tooltip;
        }
      }
    }
    return codes;
  }

  if (not obs1)
    return QVariant();
  ObsData_p obs2 = getObs2(obs1);

  if (role == Qt::FontRole) {
    QFont f;
#if 0
    if ((obs1 and obs1->modified()) or (obs2 and obs2->modified()))
      f.setBold(true);
#endif
    return f;
  }

  int i=0;
  Codes_t codes = getCodes(obs1, obs2);
  if (codes.first >= 0) {
    for(; vxdata and vxdata[i].code >= 0; ++i)
      if (codes.first == vxdata[i].code)
        break;
  }

  if (role == Qt::DisplayRole or role == Qt::EditRole) {
    if (codes.first == kvalobs::NEW_ROW)
      return qApp->translate("Code2Text", "new");
    else if (codes.first == kvalobs::MISSING)
      return qApp->translate("Code2Text", "mis");
    else if (codes.first == kvalobs::REJECTED)
      return qApp->translate("Code2Text", "rej");

    if (not vxdata or vxdata[i].code < 0) {
      if (codes.second == -1)
        return QString("%1?").arg(codes.first);
      else
        return QString("%1/%2?").arg(codes.first).arg(codes.second);
    }

    QString display = vxdata[i].metCode;
    if (codes.first != 0) {
      if (codes.second == 0)
        display += QChar( 0xB0 );
      else if (codes.second == 2)
        display += QChar( 0xB2 );
    }
    return display;
  } else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
    QString tooltip;
    if (not vxdata or vxdata[i].code < 0) {
      if (codes.second == -1)
        tooltip = qApp->translate("DataVxItem", "unknown code %1").arg(codes.first);
      else
        tooltip = qApp->translate("DataVxItem", "unknown code %1 (strength %2)").arg(codes.first).arg(codes.second);
    } else {
      tooltip = qApp->translate("DataVxItem", vxdata[i].explain);
      if (codes.first != 0 and (codes.second == 0 or codes.second == 2)) {
        tooltip += " -- ";
        if (codes.second == 0)
          tooltip += qApp->translate("DataVxItem", "weak");
        else
          tooltip += qApp->translate("DataVxItem", "strong");
      }
      QString codeNumber = QString::number(codes.first);
      if (codes.second >= 0)
        codeNumber += "/" + QString::number(codes.second);
      Helpers::appendText(tooltip, "(" + codeNumber + ")", " ");
    }
#if 0
    Helpers::appendText(tooltip, tasks::asText(obs1->allTasks()));
#endif
    return tooltip;
  }
  return DataValueItem::data(obs1, st, role);
}

bool DataVxItem::setData(ObsData_p obs1, EditAccess_p, const SensorTime& st, const QVariant& value, int role)
{
  if (role != Qt::EditRole or mColumnType != ObsColumn::NEW_CORRECTED)
    return false;

  METLIBS_LOG_SCOPE();
  const VxData* vxdata = vxData4SensorTime(st);
  if (not vxdata) {
    HQC_LOG_WARN("no Vx codes known for " << st);
    return false;
  }

  ObsData_p obs2 = getObs2(obs1);
  const Codes_t oldCodes = getCodes(obs1, obs2);
  
  const QString v = value.toString();
  METLIBS_LOG_DEBUG(LOGVAL(v));
  if (v == "") {
    mDA->newVersion();
    bool changed = false;
    if (obs1) {
      ObsUpdate_p update = mDA->createUpdate(obs1);
      update->setCorrected(0);
      mDA->storeUpdates(ObsUpdate_pv(1, update));
      changed = true;
    }
    if (obs2 and oldCodes.second != 1) {
      ObsUpdate_p update = mDA->createUpdate(obs2);
      update->setCorrected(1);
      mDA->storeUpdates(ObsUpdate_pv(1, update));
      changed = true;
    }
    return changed;
  }
  
  if (v.length() != 3 and v.length() != 2)
    return false;
  const QString mc = v.left(2), level = v.mid(2);

  int i=1; // start at 1, skipping "no data"
  for(; vxdata[i].code >= 0; ++i) {
    if (mc == vxdata[i].metCode)
      break;
  }
  METLIBS_LOG_DEBUG(LOGVAL(vxdata[i].code));
  if (vxdata[i].code < 0)
    return false;
  const int newCode1 = vxdata[i].code;

  int newCode2;
  if (level == "" or level == " " or level == "1")
    newCode2 = 1;
  else if (level == QChar(0xB0) or level == "0")
    newCode2 = 0;
  else if (level == QChar(0xB2) or level == "2")
    newCode2 = 2;
  else
    return false;

  METLIBS_LOG_DEBUG(LOGVAL(oldCodes.first) << LOGVAL(newCode1) << LOGVAL(oldCodes.second) << LOGVAL(newCode2));
  bool pushed = false;
  if (newCode1 != oldCodes.first) {
    mDA->newVersion();
    pushed = true;
    ObsUpdate_p update = obs1 ? mDA->createUpdate(obs1) : mDA->createUpdate(st);
    Helpers::auto_correct(update, obs1, newCode1);
    mDA->storeUpdates(ObsUpdate_pv(1, update));
    METLIBS_LOG_DEBUG(LOGVAL(update->corrected()));
  }
  if (newCode2 != oldCodes.second) {
    if (not pushed)
      mDA->newVersion();
    ObsUpdate_p update;
    if (obs2) {
      update = mDA->createUpdate(obs2);
    } else {
      SensorTime st2(st);
      st2.sensor.paramId += 1;
      update = mDA->createUpdate(st2);
    }
    Helpers::auto_correct(update, obs2, newCode2);
    mDA->storeUpdates(ObsUpdate_pv(1, update));
    METLIBS_LOG_DEBUG(LOGVAL(update->corrected()));
  }
  return true;
}


bool DataVxItem::matchSensor(const Sensor& sensorColumn, const Sensor& sensorObs) const
{
  return (eq_Sensor()(sensorColumn, sensorObs)
      or eq_Sensor()(getSensor2(sensorColumn), sensorObs));
}

QString DataVxItem::description(bool mini) const
{
  const bool orig = (mColumnType == ObsColumn::ORIGINAL);
  if (mini) {
    return orig ? qApp->translate("DataColumn", "orig")
        : qApp->translate("DataColumn", "corr");
  } else {
    return orig ? qApp->translate("DataColumn", "original")
        : qApp->translate("DataColumn", "corrected");
  }
}

DataVxItem::Codes_t DataVxItem::getCodes(ObsData_p obs1, ObsData_p obs2) const
{
  int v1 = 0, v2 = -1;
  if (mColumnType != ObsColumn::ORIGINAL) {
    if (obs1)
      v1 = static_cast<int>(obs1->corrected());
    if (obs2)
      v2 = static_cast<int>(obs2->corrected());
  } else {
    if (obs1)
      v1 = static_cast<int>(obs1->original());
    if (obs2)
      v2 = static_cast<int>(obs2->original());
  }
  return std::make_pair(v1, v2);
}

ObsData_p DataVxItem::getObs2(ObsData_p obs1) const
{
#if 0
  if (not obs1)
    return ObsData_p();
  SensorTime st2(obs1->sensorTime());
  st2.sensor.paramId += 1;
  return mDA->findE(st2);
#else
  return ObsData_p();
#endif
}

Sensor DataVxItem::getSensor2(const Sensor& sensor1) const
{
  Sensor sensor2(sensor1);
  sensor2.paramId += 1;
  return sensor2;
}
