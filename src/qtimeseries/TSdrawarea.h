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


#ifndef _tsDrawArea_h
#define _tsDrawArea_h

#include "TSPlot.h"

#include <map>
#include <vector>

#include <puTools/miTime.h>

#include <pets2/ptGlobals.h>
#include <pets2/ptDiagram.h>
#include <pets2/ptPlotElement.h>

#include <tsData/ptWeatherParameter.h>

class TSdrawarea {
private:
  pets2::ptCanvas* canvas;
  pets2::ptDiagram * diagram;
  ptDiagramData * petsdata;
  pets2::ptStyle diaStyle;
  TimeSeriesData::TSPlot tsplot;

  typedef std::map<std::string, TimeSeriesData::ts_time_t> timemarks_t;
  timemarks_t timemarks;
  int plotorder;
  bool Initialised;

  void useTimemarks();
  bool prepareData();
  bool prepareDiagram();
  void fillElement(pets2::Primitive& p, pets2::Layout& l, const POptions::PlotOptions& opt);
  pets2::ptColor pets_colour(const POptions::Colour& col);
  pets2::ptLineStyle pets_linestyle(const POptions::Linetype& lt);
  pets2::ptFillStyle pets_fillstyle(const POptions::Filltype& ft);

public:
  TSdrawarea();
  ~TSdrawarea();

  void prepare(const TimeSeriesData::TSPlot& tsp);

  void setViewport(pets2::ptCanvas* canvas);

  void plot(pets2::ptPainter& painter);

  void setTimemark(const TimeSeriesData::ts_time_t& nt, const std::string& name="");
  void clearTimemarks(const std::string& name="");
};

#endif // _tsDrawArea_h
