/*
  libqTimeseries - Qt classes for time series plots

  Copyright (C) 2006-2018 met.no

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "TSglwidget.h"

#include <QPaintEvent>
#include <QResizeEvent>

#define MILOGGER_CATEGORY "qtimeseries.TSglwidget"
#include <miLogger/miLogging.h>

TSglwidget::TSglwidget(QWidget* parent, const char* name)
    : QWidget(parent)
    , canvas(this)
{
  setObjectName(name);
}

void TSglwidget::paintEvent(QPaintEvent*)
{
  METLIBS_LOG_SCOPE(LOGVAL(width()) << LOGVAL(height()));

  pets2::ptQPainter painter(&canvas);
  drawArea.plot(painter);
}

void TSglwidget::resizeEvent(QResizeEvent*)
{
  METLIBS_LOG_SCOPE();
  canvas.update();
  drawArea.setViewport(&canvas);
}

void TSglwidget::prepare(const TimeSeriesData::TSPlot& tsp)
{
  drawArea.prepare(tsp);
  update();
}


void TSglwidget::refresh()
{
  update();
}

#if 0
void TSglwidget::hardcopy(const printOptions& p)
{
  drawArea.startHardcopy(p);
  refresh();
}
#endif

void TSglwidget::setTimemark(const miutil::miTime& tim, const std::string& nam)
{
  drawArea.setTimemark(tim, nam);
  refresh();
}

void TSglwidget::clearTimemarks(const std::string& nam)
{
  drawArea.clearTimemarks(nam);
  refresh();
}
