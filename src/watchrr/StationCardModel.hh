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

#ifndef WATCHRR_STATIONCARDMODEL_HH
#define WATCHRR_STATIONCARDMODEL_HH 1

#include "EditTimeColumn.hh"
#include "WatchRRTableModel.hh"
#include "common/ObsColumn.hh"
#include "common/ModelAccess.hh"

class StationCardModel : public WatchRRTableModel
{
public:
  StationCardModel(TaskAccess_p da, ModelAccess_p ma, const Sensor& sensor, const TimeSpan& time);

  int getRR24CorrectedColumn() const;
  int getRR24OriginalColumn() const;
  void setRR24TimeSpan(const TimeSpan& tr);

private:
  void addStationColumn(const Sensor& s, int paramId, ObsColumn::Type columnType, int timeOffset);

private:
  std::shared_ptr<EditTimeColumn> mRR24EditTime;
  int mRR24CorrectedColumn;
  int mRR24OriginalColumn;
};

#endif /* WATCHRR_STATIONCARDMODEL_HH */
