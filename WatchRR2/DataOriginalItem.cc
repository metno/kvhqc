
#include "DataOriginalItem.hh"

#include "Helpers.hh"

#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QBrush>

#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.DataOriginalItem"
#include "HqcLogging.hh"

DataOriginalItem::DataOriginalItem(Code2TextCPtr codes)
  : DataValueItem(ObsColumn::ORIGINAL)
  , mCodes(codes)
{
}

QVariant DataOriginalItem::data(EditDataPtr obs, int role) const
{
  if (not obs)
    return QVariant();
  
  if (role == Qt::ForegroundRole) {
    if (mCodes->isCode(getValue(obs)))
      return Qt::darkGray;
  } else if (role == Qt::DisplayRole) {
    return mCodes->asText(getValue(obs));
  } else if (role == Qt::TextAlignmentRole) {
    return Qt::AlignVCenter+(mCodes->isCode(getValue(obs)) ? Qt::AlignLeft : Qt::AlignRight);
  } else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
    QString tip = mCodes->asTip(getValue(obs));
    const int ui_2 = Helpers::extract_ui2(obs);
    if (ui_2 == 3)
      Helpers::appendText(tip, qApp->translate("DataOriginalItem", "surely wrong"));
    else if (ui_2 == 2)
      Helpers::appendText(tip, qApp->translate("DataOriginalItem", "very suspicious (probably wrong)"));
    else if (ui_2 == 1)
      Helpers::appendText(tip, qApp->translate("DataOriginalItem", "suspicious (probably ok)"));
    else if (ui_2 == 9)
      Helpers::appendText(tip, qApp->translate("DataOriginalItem", "no quality info available"));
    return tip;
  }
  return DataValueItem::data(obs, role);
}

QString DataOriginalItem::description(bool mini) const
{
  if (mini)
    return qApp->translate("DataColumn", "orig");
  else
    return qApp->translate("DataColumn", "original");
}
