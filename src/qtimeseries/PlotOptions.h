/*
  libqTimeseries - Qt classes for time series plots
  
  Copyright (C) 2006 met.no

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


#ifndef QTIMESERIES_PLOTOPTIONS_H
#define QTIMESERIES_PLOTOPTIONS_H

#include "Colour.h"
#include "Linetype.h"
#include <vector>

namespace POptions {

/* Created by met.no/FoU/PU 
   at Wed Jan 22 08:49:58 2003 */

enum polyStyle {
  poly_fill,
  poly_border,
  poly_both,
  poly_none
};

enum Alignment {
  align_left,
  align_right,
  align_top,
  align_bottom,
  align_center
};

enum yAxis {
  axis_left_left,   // left side of left axis
  axis_left_right,  // right side of left axis
  axis_right_left,  // left side of right axis
  axis_right_right  // right side of right axis
};

struct Filltype {
  std::string name;
  unsigned int bmap;
};

enum PlotType {
  type_line,        // curve in axis
  type_histogram,   // histogram in axis
  type_vector,      // vector (one (dir) or two (dir,strength) data-values)
  type_wind_vector  // standard wind-vector (req. DD and FF)
};

enum Marker {
  NO_MARKER,
  M_RECTANGLE,
  M_TRIANGLE,
  M_DIAMOND,
  M_STAR,
  M_CIRCLE
};

// time-elements (add to time_types below)
extern const char T_HOUR[];
extern const char T_DAY[];
extern const char T_DAYNUM[];
extern const char T_DATE[];
extern const char T_WEEK[];
extern const char T_MONTH[];
extern const char T_MONTHNUM[];
extern const char T_YEAR[];

  
struct PlotOptions {
  std::string name;      // name on plot
  std::string label;     // label
  std::string axisname;  // axis-name
  std::vector<std::string> time_types; // list of time-elements
  bool drawgrid;              // draw grid in diagram
  int gridTimes;              // draw grid on hours modulo gridTimes
  std::string language;  // "NO" or "EN"

  PlotType plottype;          // type of plotelement 
  Colour textcolour;          // various colours: main title etc.
  Colour linecolour;          // ..of lines
  Colour fillcolour;          // ..of fillpattern
  Colour bordercolour;        // ..of borders on some elements
  Colour timecolour;          // ..of time-elements

  Linetype linetype;          // dashed, solid, etc.
  int linewidth;              // width of lines
  Filltype filltype;          // pattern for filling

  //polyStyle polystyle;
  //miutil::miString  fontname;
  //miutil::miString  fontface;
  //float     fontsize;

  // ---- applies to curves and histograms
  yAxis axis;                 // which axis to use (for curves)
  bool axisFixedMin;          // fixed min value for axis
  float axisMin;              // min value for axis
  bool axisFixedMax;          // fixed max value for axis
  float axisMax;              // max value for axis
  Marker marker;              // type of marker for lines

  PlotOptions();
};

} // namespace POptions

#endif // QTIMESERIES_PLOTOPTIONS_H
