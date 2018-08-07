/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2014-2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "SensorHeader.hh"

#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "util/stringutil.hh"

#include <QCoreApplication>

SensorHeader::SensorHeader(const Sensor& sensor, When showStation, When showParam, int timeOffset)
  : mSensor(sensor)
  , mShowStation(mSensor.stationId >= 0 ? showStation : NEVER)
  , mShowParam(mSensor.paramId >= 0 ? showParam : NEVER)
  , mTimeOffset(timeOffset)
{
}

QVariant SensorHeader::sensorHeader(DataItem_p item, Qt::Orientation orientation, int role) const
{
  const bool tooltip = (role == Qt::ToolTipRole), display = (role == Qt::DisplayRole);
  if (not tooltip and not display)
    return QVariant();

  QString header;
  if (tooltip) {
    if (mShowStation != NEVER) {
      header = stationTooltip();
      if (mSensor.sensor != 0 || mSensor.level != 0)
        Helpers::appendText(header, qApp->translate("SensorHeader", "Sensor %1 Level %2").arg(mSensor.sensor).arg(mSensor.level));
      if (mSensor.typeId != 0)
        Helpers::appendText(header, qApp->translate("SensorHeader", "Type %1").arg(mSensor.typeId));
    }
    if (mShowParam != NEVER) {
      Helpers::appendText(header, qApp->translate("SensorHeader", "Parameter %1")
          .arg(KvMetaDataBuffer::instance()->paramName(mSensor.paramId)), "\n");
      if (item)
        Helpers::appendText(header, item->description(false), " ");
    }
  } else {
    header = displayHeader(orientation, item ? item->description(true) : "");
  }
  return header + timeOffset(orientation, role);
}

QVariant SensorHeader::modelHeader(Qt::Orientation orientation, int role) const
{
  const QVariant sh = sensorHeader(DataItem_p(), orientation, role);
  if (not sh.isValid())
    return sh;
  const bool tooltip = (role == Qt::ToolTipRole);
  const QString sep = tooltip ? QString(" ") : separator(orientation);
  return Helpers::appendedText(sh.toString(), qApp->translate("SensorHeader", "model"), sep);
}

QString SensorHeader::displayHeader(Qt::Orientation orientation, const QString& description) const
{
  QString header;
  if (mShowStation == ALWAYS)
    header = QString::number(mSensor.stationId);
  if (mShowParam == ALWAYS) {
    const QString sep = separator(orientation);
    Helpers::appendText(header, KvMetaDataBuffer::instance()->paramName(mSensor.paramId), sep);
    if (mSensor.typeId != 0)
      Helpers::appendText(header, QString("T%1").arg(mSensor.typeId), sep);
    if (mSensor.sensor != 0 or mSensor.level != 0)
      Helpers::appendText(header, QString("L%1 S%2").arg(mSensor.level).arg(mSensor.sensor), sep);
    Helpers::appendText(header, description, sep);
  }
  return header;
}

QString SensorHeader::timeOffset(Qt::Orientation orientation, int role) const
{
  if (mTimeOffset == 0)
    return "";

  const QString sep = separator(orientation);
  const bool mini = (role == Qt::DisplayRole);
  if (mTimeOffset > 0) {
    return sep + (mini ? qApp->translate("SensorHeader", "+%1h")
        : qApp->translate("SensorHeader", "time offset +%1 hour(s)"))
        .arg(mTimeOffset);
  } else {
    return sep + (mini ? qApp->translate("SensorHeader", "-%1h")
        : qApp->translate("SensorHeader", "time offset -%1 hour(s)"))
        .arg(-mTimeOffset);
  }
}

QString SensorHeader::separator(Qt::Orientation orientation) const
{
  return (orientation == Qt::Horizontal) ? "\n" : " ";
}

QString SensorHeader::stationTooltip() const
{
  try {
    const kvalobs::kvStation& s = KvMetaDataBuffer::instance()->findStation(mSensor.stationId);
    return qApp->translate("SensorHeader", "Station %1 [%2, %3masl]").arg(mSensor.stationId).arg(Helpers::stationName(s)).arg(s.height(), 0);
  } catch (std::exception& ) {
    return QString("?%1?").arg(mSensor.stationId);
  }
}
