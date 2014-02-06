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


#ifndef _TSData_h
#define _TSData_h

#include "PlotOptions.h"
#include <puTools/miTime.h>
#include <vector>


namespace TimeSeriesData {
  
/* Created by met.no/FoU/PU 
   at Wed Jan 22 16:26:55 2003 */

/*
  Useful typedefs: list of floats's with iterators
*/
typedef std::vector<float> ValueList;
typedef std::vector<float>::iterator IValueList;
typedef std::vector<float>::const_iterator CIValueList;

typedef miutil::miTime ts_time_t;

/*
  values/time pair for use in TimeSeries
  - multiple datavalues for each time
  (convenient for vector-plots of wind etc.)
*/

class Data {
private:
  ValueList values_;
  ts_time_t time_;

public:
  Data(){}

  Data(const ts_time_t& t, const float& v)
    : time_(t) {values_.push_back(v);}

  Data(const ts_time_t& t, const float& v1, const float& v2)
    : time_(t) {values_.push_back(v1);values_.push_back(v2);}

  void clear()
    { values_.clear(); }

  void set(const ts_time_t& t, const ValueList& v)
    {time_= t; values_= v;}

  void value(const float& v)
    { values_.push_back(v); }

  void values(const ValueList& v)
    { values_= v; }

  void time(const ts_time_t& t)
    { time_=t; }

  bool empty() const
    {return values_.empty(); }

  size_t size() const
    { return values_.size(); }

  const ValueList& values() const
    { return values_; }

  float value(size_t i) const
    { return (i<values_.size()) ? values_[i] : 0; }

  const ts_time_t& time() const
    { return time_; }
};

/*
  Useful typedefs: list of Value's with iterators
*/
typedef std::vector<Data> DataList;
typedef std::vector<Data>::iterator IDataList;
typedef std::vector<Data>::const_iterator CIDataList;

} // namespace TimeSeriesData

#endif // _TSData_h
