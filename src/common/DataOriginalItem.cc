
#include "DataOriginalItem.hh"

#include "ObsHelpers.hh"
#include "util/Helpers.hh"

#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QBrush>

#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.DataOriginalItem"
#include "util/HqcLogging.hh"

DataOriginalItem::DataOriginalItem(Code2TextCPtr codes)
  : DataCodeItem(ObsColumn::ORIGINAL, codes)
{
}

QVariant DataOriginalItem::data(EditDataPtr obs, const SensorTime& st, int role) const
{
  if (not obs)
    return QVariant();
  
  if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
    QString tip;
    const int ui_2 = Helpers::extract_ui2(obs);
    if (ui_2 == 3)
      tip = qApp->translate("DataOriginalItem", "surely wrong");
    else if (ui_2 == 2)
      tip = qApp->translate("DataOriginalItem", "very suspicious (probably wrong)");
    else if (ui_2 == 1)
      tip = qApp->translate("DataOriginalItem", "suspicious (probably ok)");
    else if (ui_2 == 9)
      tip = qApp->translate("DataOriginalItem", "no quality info available");
    return Helpers::appendedText(tip, DataCodeItem::data(obs, st, role).toString());
  }
  return DataCodeItem::data(obs, st, role);
}

QString DataOriginalItem::description(bool mini) const
{
  if (mini)
    return qApp->translate("DataColumn", "orig");
  else
    return qApp->translate("DataColumn", "original");
}
