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


#ifndef _TimeSeries_h
#define _TimeSeries_h


/* Created by met.no/FoU/PU 
   at Wed Jan 22 16:30:17 2003 */

#include "TSData.h"
#include <set>

namespace TimeSeriesData {
  
/* Created by met.no/FoU/PU 
   at Tue Jan 21 16:52:16 2003 */

/*
  TimeSeries: a list of Data's with station,parameter,level
  info and plotoptions
  - this is a proper timeseries (one or more datavalues for
  each time), and should be used for one single graphical
  element (curve, vector, wind-vector etc)
  - all graphical layout/options specified in the PlotOptions
  class
*/

class TimeSeries {
private:
  int stationid_;
  int paramid_;
  int level_;
  // List of datavalues/times
  DataList datalist_;
  // These PlotOptions apply to this timeseries..
  POptions::PlotOptions plotoptions_;

public:
  TimeSeries():stationid_(0),paramid_(0),level_(0){}
  TimeSeries(const int s, const int p, const int l)
    : stationid_(s), paramid_(p), level_(l) {}

  int stationid() const {return stationid_;}
  int paramid() const {return paramid_;}
  int level() const {return level_;}
  const DataList& values() const
    { return datalist_; }
  const POptions::PlotOptions& plotoptions() const
    { return plotoptions_; }

  void stationid(int s)  {stationid_= s;}
  void paramid(int p) {paramid_= p;}
  void level(int l) {level_= l;}
  void plotoptions(const POptions::PlotOptions& po) {plotoptions_= po;}
  void values(const DataList& v){datalist_= v;}
  void add(const Data& v){datalist_.push_back(v);}

  void clear(){datalist_.clear();}

  // check sanity of datalist
  bool dataOK() const;

  // return unique list of times in this series
  std::set<ts_time_t> times() const;
};

/*
  Useful typedefs: list of TimeSeries's with iterators
*/
typedef std::vector<TimeSeries> tsList;
typedef std::vector<TimeSeries>::iterator ItsList;
typedef std::vector<TimeSeries>::const_iterator CItsList;

} // namespace TimeSeriesData

#endif // _TimeSeries_h
