
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
