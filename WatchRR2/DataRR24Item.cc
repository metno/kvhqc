
#include "DataRR24Item.hh"

#include "Helpers.hh"

#include <QtCore/QVariant>
#include <QtGui/QBrush>

#define MILOGGER_CATEGORY "kvhqc.DataRR24Item"
#include "HqcLogging.hh"

DataRR24Item::DataRR24Item(bool showNew, Code2TextPtr codes)
    : DataCorrectedItem(showNew, codes)
{
}

Qt::ItemFlags DataRR24Item::flags(EditDataPtr obs) const
{
  Qt::ItemFlags f = DataCorrectedItem::flags(obs);
  if (obs) {
    const int typeId = obs->sensorTime().sensor.typeId;
    if (typeId <= 0 or typeId == 302 or typeId == 402)
      f &= ~Qt::ItemIsEditable;
  }
  return f;
}

QVariant DataRR24Item::data(EditDataPtr obs, int role) const
{
  const QVariant d = DataCorrectedItem::data(obs, role);
  if (role == Qt::BackgroundRole and mShowNew and not d.isValid() and obs and Helpers::is_accumulation(obs))
    return QBrush(Helpers::is_endpoint(obs) ? QColor(0xC0, 0xFF, 0xC0) : QColor(0xE0, 0xFF, 0xE0));
  return d;
}
