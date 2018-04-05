/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2013-2018 met.no

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

#include "NeighborRR24Model.hh"

#include "common/ColumnFactory.hh"
#include "common/KvHelpers.hh"
#include "common/NeighborHeader.hh"
#include "common/ObsPgmRequest.hh"

#define MILOGGER_CATEGORY "kvhqc.NeighborRR24Model"
#include "util/HqcLogging.hh"

NeighborRR24Model::NeighborRR24Model(TaskAccess_p da, const Sensor& sensor, const TimeSpan& time)
    : WatchRRTableModel(da)
    , mNeighbors(1, sensor)
{
  setTimeSpan(time);

  hqc::int_s stationIds = KvMetaDataBuffer::instance()->findNeighborStationIds(sensor.stationId);
  stationIds.insert(sensor.stationId);

  std::unique_ptr<ObsPgmRequest> op(new ObsPgmRequest(stationIds));
  op->sync();

  KvMetaDataBuffer::instance()->addNeighbors(mNeighbors, sensor, time, op.get(), 20);
  for (const Sensor& s : mNeighbors) {
    if (DataColumn_p oc = ColumnFactory::columnForSensor(da, s, time, ObsColumn::ORIGINAL))
      addColumn(oc);
  }
}

QVariant NeighborRR24Model::columnHeader(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::ToolTipRole)
    return ObsTableModel::columnHeader(section, orientation, role);

  return NeighborHeader::headerData(mNeighbors[0].stationId, mNeighbors[section].stationId, orientation, role);
}
