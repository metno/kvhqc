/*
  libqTimeseries - Qt classes for time series plots

  Copyright (C) 2006-2020 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "TimeSeries.h"

namespace TimeSeriesData {

bool TimeSeries::dataOK() const
{
  if (datalist_.size() < 2 || datalist_.front().empty())
    return false;
  CIDataList p1 = datalist_.begin();
  const size_t size0 = p1->size();
  for (++p1; p1!= datalist_.end(); ++p1)
    if (size0 != p1->size())
      return false;

  return true;
}

std::set<ts_time_t> TimeSeries::times() const
{
  std::set<ts_time_t> st;
  for (CIDataList p = datalist_.begin(); p!= datalist_.end(); p++)
    st.insert(p->time());
  return st;
}

} // namespace TimeSeriesData
