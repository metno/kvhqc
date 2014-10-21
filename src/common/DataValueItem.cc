
#include "DataValueItem.hh"

#include "HqcUserConfig.hh"
#include "KvMetaDataBuffer.hh"
#include "ObsColumn.hh"
#include "ObsHelpers.hh"
#include "Tasks.hh"
#include "HqcApplication.hh"
#include "KvHelpers.hh"

#include <kvalobs/kvDataOperations.h>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <QApplication>
#include <QBrush>
#include <QFont>
#include <QVariant>

#define MILOGGER_CATEGORY "kvhqc.DataValueItem"
#include "util/HqcLogging.hh"

DataValueItem::DataValueItem(ObsColumn::Type columnType)
  : mColumnType(columnType)
{
}

int DataValueItem::type() const
{
  return mColumnType;
}

Qt::ItemFlags DataValueItem::flags(ObsData_p obs) const
{
    Qt::ItemFlags f = DataItem::flags(obs);
    if (mColumnType == ObsColumn::NEW_CORRECTED)
        f |= Qt::ItemIsEditable;
    return f;
}

QVariant DataValueItem::data(ObsData_p obs, const SensorTime& st, int role) const
{
  if (not obs)
    return QVariant();
  
  const bool isNC = mColumnType == ObsColumn::NEW_CORRECTED;
  if (role == Qt::BackgroundRole) {
    if (isNC) {
#if 0
      if (obs->hasRequiredTasks())
        return QBrush(Qt::red);
      else if (obs->hasTasks())
        return QBrush(QColor(0xFF, 0x60, 0)); // red orange
#endif
    } else if (mColumnType == ObsColumn::ORIGINAL) {
      const int ui_2 = Helpers::extract_ui2(obs);
      if (hqcApp) {
        const QColor bg = hqcApp->userConfig()->dataOrigUI2Background(ui_2);
        if (bg.isValid())
          return QBrush(bg);
      }
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
      const kvalobs::kvControlInfo& ci = obs->controlinfo();
      if (ci.flag(kvalobs::flag::fhqc) == 0) { // not hqc touched
        if (ci.qc2dDone())
          return Qt::darkMagenta;
        else if (ci.flag(kvalobs::flag::fnum) >= 6)
          return Qt::red;
      }
    }
  } else if (role == Qt::FontRole) {
    QFont f;
    if (obs->isModified())
      f.setBold(true);
    return f;
  }
  return DataItem::data(obs, st, role);
}

float DataValueItem::getValue(ObsData_p obs) const
{
  if (not obs)
    return kvalobs::MISSING;
  if (mColumnType == ObsColumn::ORIGINAL)
    return obs->original();
  else
    return obs->corrected();
}
