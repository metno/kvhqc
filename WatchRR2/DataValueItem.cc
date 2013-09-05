
#include "DataValueItem.hh"

#include "Helpers.hh"
#include "KvMetaDataBuffer.hh"
#include "ObsColumn.hh"
#include "Tasks.hh"

#include <kvalobs/kvDataOperations.h>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QBrush>

#define MILOGGER_CATEGORY "kvhqc.DataValueItem"
#include "HqcLogging.hh"

DataValueItem::DataValueItem(ObsColumn::Type columnType)
  : mColumnType(columnType)
{
}

int DataValueItem::type() const
{
  return mColumnType;
}

Qt::ItemFlags DataValueItem::flags(EditDataPtr obs) const
{
    Qt::ItemFlags f = DataItem::flags(obs);
    if (mColumnType == ObsColumn::NEW_CORRECTED)
        f |= Qt::ItemIsEditable;
    return f;
}

QVariant DataValueItem::data(EditDataPtr obs, const SensorTime& st, int role) const
{
  if (not obs)
    return QVariant();
  
  const bool isNC = mColumnType == ObsColumn::NEW_CORRECTED;
  if (role == Qt::BackgroundRole) {
    if (isNC) {
      if (obs->hasRequiredTasks())
        return QBrush(Qt::red);
      else if (obs->hasTasks())
        return QBrush(QColor(0xFF, 0x60, 0)); // red orange
    } else if (mColumnType == ObsColumn::ORIGINAL) {
      const int ui_2 = Helpers::extract_ui2(obs);
      if (ui_2 == 1)      // probably ok
        return QBrush(QColor(0xFF, 0xFF, 0xF0)); // light yellow
      else if (ui_2 == 2) // probably wrong
        return QBrush(QColor(0xFF, 0xF0, 0xF0)); // light red
      else if (ui_2 == 9) // no quality information
        return QBrush(QColor(0xFF, 0xE0, 0xB0)); // light orange
      else if (ui_2 != 0) // wrong
        return QBrush(QColor(0xFF, 0xE0, 0xE0)); // light red
    }
  } else if (role == Qt::ForegroundRole) {
    // FIXME this is a hack, but the idea of having all non-numbers in dark gray is also mysterious
    const QVariant d = data(obs, st, Qt::DisplayRole);
    if (d.type() == QVariant::String) {
      bool ok = false;
      d.toFloat(&ok);
      if (!ok)
        return Qt::darkGray;
    }
    if (mColumnType != ObsColumn::ORIGINAL) {
      const kvalobs::kvControlInfo ci(isNC ? obs->controlinfo() : obs->oldControlinfo());
      if (ci.flag(kvalobs::flag::fhqc) == 0) { // not hqc touched
        if (ci.qc2dDone())
          return Qt::darkMagenta;
        else if (ci.flag(kvalobs::flag::fnum) >= 6)
          return Qt::red;
      }
    }
  } else if (role == Qt::FontRole) {
    QFont f;
    if (mColumnType == ObsColumn::NEW_CORRECTED and obs->modifiedCorrected())
      f.setBold(true);
    return f;
  }
  return DataItem::data(obs, st, role);
}

float DataValueItem::getValue(EditDataPtr obs) const
{
  if (not obs)
    return kvalobs::MISSING;
  if (mColumnType == ObsColumn::OLD_CORRECTED)
    return obs->oldCorrected();
  else if (mColumnType == ObsColumn::ORIGINAL)
    return obs->original();
  else
    return obs->corrected();
}
