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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "TSglwidget.h"

const float gl_width =  1500.0;
const float gl_height = 1000.0;

TSglwidget::TSglwidget(QWidget* parent, const char* name)
  : QGLWidget(parent)
{
  setObjectName(name);
}

void TSglwidget::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT);
  //glLoadIdentity();
  drawArea.plot();
}



//  Set up the OpenGL rendering state
void TSglwidget::initializeGL()
{
  glOrtho(0,gl_width,0,gl_height,-1,1);
  glShadeModel( GL_FLAT );
  glClearColor(1.0,1.0,1.0,1.0);
}


//  Set up the OpenGL view port, matrix mode, etc.
void TSglwidget::resizeGL( int w, int h )
{
  glViewport( 0, 0, (GLint)w, (GLint)h );
  plotw= w;
  ploth= h;

  float pw = gl_width/float(plotw);
  float ph = gl_height/float(ploth);

  drawArea.setViewport(w,h,pw,ph);
}

void TSglwidget::prepare(const TimeSeriesData::TSPlot& tsp)
{
  makeCurrent(); // set current OpenGL context
  drawArea.prepare(tsp);
  updateGL();
}


void TSglwidget::refresh()
{
  updateGL();
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
