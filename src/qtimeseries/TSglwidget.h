/*
  libqTimeseries - Qt classes for time series plots

  Copyright (C) 2006-2013 met.no

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

#ifndef QTIMESERIES_TSGLWIDGET_H
#define QTIMESERIES_TSGLWIDGET_H

#include "TSdrawarea.h"
#include <pets2/ptQPainter.h>

#include <QWidget>

/* Created by met.no/FoU/PU
   at Thu Feb 13 16:19:41 2003 */

class TSglwidget : public QWidget
{
  Q_OBJECT

private:
  int plotw;
  int ploth;
  TSdrawarea drawArea;
  pets2::ptQCanvas canvas;

protected:
  void resizeEvent(QResizeEvent*);
  void paintEvent(QPaintEvent*);

public:
  TSglwidget(QWidget* parent, const char* name="TSglWidget");

  void prepare(const TimeSeriesData::TSPlot& tsp);
  void refresh();
  void setTimemark(const TimeSeriesData::ts_time_t&, const std::string& = "");
  void clearTimemarks(const std::string& = "");
};

#endif // QTIMESERIES_TSGLWIDGET_H
