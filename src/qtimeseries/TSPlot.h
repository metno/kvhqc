/*
  libqTimeseries - Qt classes for time series plots
  
  Copyright (C) 2006-2014 met.no

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

#ifndef QTIMESERIES_TSPLOT_H
#define QTIMESERIES_TSPLOT_H

#include "TimeSeries.h"
#include "PlotOptions.h"

#include <set>

namespace TimeSeriesData {

/*
  Several timeseries for one Plot
*/
class TSPlot {
private:
  // the list of timeseries
  tsList tserieslist_;
  // These PlotOptions apply to the whole plot..
  // ex: fillcolour = background colour
  POptions::PlotOptions plotoptions_;

public:
  const tsList& tserieslist() const
    { return tserieslist_; }

  const POptions::PlotOptions& plotoptions() const
    { return plotoptions_; }

  // return union of times for all timeseries
  std::set<miutil::miTime> times() const
    {
      std::set<miutil::miTime> st;
      for (CItsList p = tserieslist_.begin(); p!= tserieslist_.end(); p++) {
	const std::set<miutil::miTime> vt= p->times();
	st.insert(vt.begin(), vt.end());
      }
      return st;
    }

  // check sanity of all datalists - return false
  // if all datalists bad
  bool dataOK() const
    {
      for (CItsList p = tserieslist_.begin(); p!= tserieslist_.end(); p++)
        if (not p->dataOK())
          return false;
      return true;
    }

  void tserieslist(const tsList& tl)
    { tserieslist_= tl; }

  void plotoptions(const POptions::PlotOptions& po)
    { plotoptions_= po; }
};

} // namespace TimeSeriesData

#endif // QTIMESERIES_TSPLOT_H
