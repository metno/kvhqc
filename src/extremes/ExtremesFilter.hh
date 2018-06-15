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


#ifndef EXTREMESFILTER_HH
#define EXTREMESFILTER_HH 1

#include "common/KvTypedefs.hh"
#include "common/ObsFilter.hh"

#include <set>

class ExtremesFilter : public ObsFilter
{
public:
  ExtremesFilter(int paramid, int nExtremes);

  QString acceptingSql(const QString& data_alias, const TimeSpan& time) const override;
  QString acceptingSqlExtraTables(const QString& data_alias, const TimeSpan& time) const override;

  bool needsSQL() const override
    { return true; }

  bool accept(ObsData_p obs, bool afterSQL) const override;

  bool subsetOf(ObsFilter_p other) const override;

  bool isMaximumSearch() const
    { return mFindMaximum; }

private:
  int mParamId;
  int mExtremesCount;

  hqc::int_s mParamIds;
  bool mFindMaximum;

  std::set<int> mExcludedIds;
};

HQC_TYPEDEF_P(ExtremesFilter);

#endif // EXTREMESFILTER_HH
