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

#include "TSdrawarea.h"

#include "puMet/miSymbol.h"
#include "puMet/symbolMaker.h"

#include "pets2/ptSymbolElement.h"
#include "pets2/ptTimemarkerElement.h"
#include "pets2/ptFontManager.h"

#include "glText/glTextQtTexture.h"

#include <Qt>

//#ifdef XFONTS
//#include <glText/glTextX.h>
//#else
//#include <glText/glTextTT.h>
//#endif

#define MILOGGER_CATEGORY "qTimeseries.TSdrawarea"
#include "miLogger/miLogging.h"

//extern
symbolMaker wsymbols;

using namespace TimeSeriesData;

TSdrawarea::TSdrawarea()
    : diagram(0)
    , petsdata(0)
    , width(1), height(1)
    , pixwidth(1), pixheight(1)
    , Initialised(false)
{
}

TSdrawarea::~TSdrawarea()
{
  delete diagram;
  delete petsdata;
}

void TSdrawarea::prepare(const TimeSeriesData::TSPlot& tsp)
{
  if (!Initialised) {
//    cout << "Preparing FONTS" << endl;
//#ifdef XFONTS
//    glText* gltext = new glTextX();
//#else
//    glText* gltext= new glTextTT();
//#endif

    glText* gltext = new glTextQtTexture();
    gltext->testDefineFonts();
    FM.addFontCollection(gltext, XFONTSET);
    FM.setFontColl(XFONTSET);
    // FIXME gltext memory leak

    Initialised = true;
  }

  tsplot = tsp;

  if (!prepareData()) {
    METLIBS_LOG_WARN("TSdrawarea::prepare PREPARE DATA failed");
    return;
  }
  if (!prepareDiagram())
    METLIBS_LOG_WARN("TSdrawarea::prepare PREPARE DIAGRAM failed");
}

void TSdrawarea::setViewport(int w, int h, float pw, float ph)
{
  width = w;
  height = h;

  glwidth = w * pw;
  glheight = h * ph;

  pixwidth = pw;
  pixheight = ph;

  if (diagram)
    diagram->setViewport(width, height, glwidth, glheight);

}

ptColor TSdrawarea::pets_colour(const POptions::Colour& col)
{
  const std::string name = miutil::to_upper(col.Name());
  return ptColor(name, col.fR(), col.fG(), col.fB(), col.fA());
}

ptLineStyle TSdrawarea::pets_linestyle(const POptions::Linetype& lt)
{
  ptLineStyle ls = FULL;

  if (lt.name == "dashed")
    ls = DASHED;
  else if (lt.name == "dashdotted")
    ls = DASHDOTTED;
  else if (lt.name == "dashdashdotted")
    ls = DASHDASHDOTTED;
  else if (lt.name == "dotted")
    ls = DOTTED;

  return ls;
}

ptFillStyle TSdrawarea::pets_fillstyle(const POptions::Filltype& ft)
{
  ptFillStyle fs = SOLID;

  if (ft.name == "none")
    fs = NONE;
  else if (ft.name == "diagright")
    fs = DIAGRIGHT;
  else if (ft.name == "diagleft")
    fs = DIAGLEFT;
  else if (ft.name == "diagcross")
    fs = DIAGCROSS;
  else if (ft.name == "horizontal")
    fs = HORIZONTAL;
  else if (ft.name == "vertical")
    fs = VERTICAL;
  else if (ft.name == "shorizontal")
    fs = SHORIZONTAL;
  else if (ft.name == "svertical")
    fs = SVERTICAL;
  else if (ft.name == "square")
    fs = SQUARE;

  return fs;
}

void TSdrawarea::fillElement(Primitive& p, Layout& l,
    const POptions::PlotOptions& opt)
{
  //static int plotorder = 1;

  p.mother = DIAGRAM;
  p.type = DUM_PRIMITIVE;
  if (opt.plottype == POptions::type_line) {
    p.type = LINE;
    p.mother = AXES1;
  } else if (opt.plottype == POptions::type_histogram) {
    p.type = AXISHIST;
    p.mother = AXES1;
  } else if (opt.plottype == POptions::type_wind_vector) {
    p.type = WIND_VECTOR;
    l.centerVector = true;
  } else if (opt.plottype == POptions::type_vector) {
    p.type = VECTOR;
    l.vectorArrow = true;
  }

  //cerr << "NAME:" << opt.name << " ORDER:" <<  plotorder << endl;
  p.order = plotorder++;
  //p.id    = ParId(opt.name,L_UNDEF,M_UNDEF,R_UNDEF,S_UNDEF);
  p.nr = 0;
  p.plotAll = false;

  l.name = opt.name;
  l.height = 75; // OBS
  l.spacing = SMALL; // OBS
  l.intSpacing = SMALL; // OBS

  ptColor textcolour = pets_colour(opt.textcolour);
  ptColor linecolour = pets_colour(opt.linecolour);
  ptColor fillcolour = pets_colour(opt.fillcolour);
  ptColor bordercolour = pets_colour(opt.bordercolour);

  l.color = linecolour;
  l.color2 = fillcolour;

  l.font = NORMAL; // OBS
  l.lineWidth = opt.linewidth;
  l.label = true; // OBS
  l.patternInColour = true; //OBS
  l.linePattern = pets_linestyle(opt.linetype);
  l.fillstyle = pets_fillstyle(opt.filltype);
  l.align = LEFT;// OBS
  l.text = opt.label;
  //l.text2=
  //l.axis

  //OBS
  l.yaid = 0;
  l.axisgrid = false;
  l.gridcolor = ptColor("GREY25");
  l.gridstyle = DOTTED;

  l.axisRectangle = true;
  l.gridxonly = false;
  l.axisWidth = 1;
  l.tickWidth = 1;
  l.tickLen = 10;

  // find line-marker
  switch (opt.marker) {
  case POptions::M_CIRCLE:
    l.marker = M_CIRCLE;
    break;
  case POptions::M_RECTANGLE:
    l.marker = M_RECTANGLE;
    break;
  case POptions::M_TRIANGLE:
    l.marker = M_TRIANGLE;
    break;
  case POptions::M_DIAMOND:
    l.marker = M_DIAMOND;
    break;
  case POptions::M_STAR:
    l.marker = M_STAR;
    break;
  case POptions::NO_MARKER:
    l.marker = NO_MARKER;
    break;
  }
  l.size = 4;
  l.markerFill = l.fillstyle;

  // find Y-axis
  if (opt.axis == POptions::axis_left_left)
    l.axis = LEFTLEFT;
  else if (opt.axis == POptions::axis_left_right)
    l.axis = LEFTRIGHT;
  else if (opt.axis == POptions::axis_right_right)
    l.axis = RIGHTRIGHT;
  else if (opt.axis == POptions::axis_right_left)
    l.axis = RIGHTLEFT;

  p.layout = l;

}

bool TSdrawarea::prepareData()
{
  // axis helper types
  std::map<ptAxis, bool> usedaxis;
  usedaxis[LEFTLEFT] = false;
  usedaxis[LEFTRIGHT] = false;
  usedaxis[RIGHTLEFT] = false;
  usedaxis[RIGHTRIGHT] = false;
  std::map<ptAxis, int> axisid;
  axisid[LEFTLEFT] = 0;
  axisid[LEFTRIGHT] = 1;
  axisid[RIGHTLEFT] = 2;
  axisid[RIGHTRIGHT] = 3;

  float label_offset = 0;
  plotorder = 1;

  // Pets structures
  Primitive onep, curp;
  Layout onel, curl;

  // clear the style-class
  diaStyle.clear();
  diaStyle.setMargins(150, 150, 50, 50);

  if (petsdata)
    delete petsdata;
  petsdata = new ptDiagramData(wsymbols);

  POptions::PlotOptions mainplotoptions, plotoptions;

  // check if we have any data
  if (!tsplot.dataOK())
    return false;

  tsList tslist = tsplot.tserieslist();
  mainplotoptions = tsplot.plotoptions();

  // fetch union of times in plot
  std::set<miutil::miTime> timesunion = tsplot.times();
  //miutil::miTime start = *(timesunion.begin());
  //miutil::miTime end = *(timesunion.rbegin());

  // diagram title
  std::string atitle = mainplotoptions.name;

  // DEFAULT PRIMITIVE/LAYOUT
  onel.label = "";
  onel.color = ptColor("BLACK");
  onel.font = NORMAL;
  onel.height = -1;
  onel.spacing = SMALL;
  onel.intSpacing = SMALL;
  onel.numTickMajor = 10;
  onel.numTickMinor = 2;
  onel.gridxonly = false;
  onel.label = true;
  onel.language = mainplotoptions.language;

  onep.type = DUM_PRIMITIVE;
  onep.id = ID_UNDEF;
  onep.mother = DIAGRAM;
  onep.plotAll = false;
  onep.order = 0;
  onep.enabled = true;
  onep.layout = onel;

  // Add common PETS-elements...

  int order = 1;

  const std::vector<std::string>& time_types = mainplotoptions.time_types;
  int time_height = 20;
  int minSkipX = 10;

  for (size_t i=0; i<time_types.size(); i++) {
    const std::string tt_upcase = miutil::to_upper(time_types[i]);
    if (tt_upcase == POptions::T_DATE) {
      // DATE
      curl = onel;
      curl.height = time_height;
      curl.minSkipX = minSkipX;
      curl.color = pets_colour(mainplotoptions.timecolour);
      curl.datestyle = DS_DATE;

      curp = onep;
      curp.type = DATE;
      curp.order = order;
      order += 10;
      curp.layout = curl;

      diaStyle.addPrimitive(curp);

    } else if (tt_upcase == POptions::T_DAY) {
      // DAY
      curl = onel;
      curl.height = time_height;
      curl.minSkipX = minSkipX;
      curl.color = pets_colour(mainplotoptions.timecolour);
      curl.asNumber = false;
      curl.datestyle = DS_DAY;

      curp = onep;
      curp.type = DATE;
      curp.order = order;
      order += 10;
      curp.layout = curl;

      diaStyle.addPrimitive(curp);

    } else if (tt_upcase == POptions::T_DAYNUM) {
      // DAYNUM
      curl = onel;
      curl.height = time_height;
      curl.minSkipX = minSkipX;
      curl.color = pets_colour(mainplotoptions.timecolour);
      curl.asNumber = true;
      curl.datestyle = DS_DAY;

      curp = onep;
      curp.type = DATE;
      curp.order = order;
      order += 10;
      curp.layout = curl;

      diaStyle.addPrimitive(curp);

    } else if (tt_upcase == POptions::T_HOUR) {
      // UTC ----------------
      curl = onel;
      curl.text = "UTC";
      curl.height = time_height;
      curl.minSkipX = minSkipX;
      curl.spacing = 30;
      curl.color = pets_colour(mainplotoptions.timecolour);

      curp = onep;
      curp.type = UTC;
      curp.order = order;
      order += 10;
      curp.layout = curl;

      diaStyle.addPrimitive(curp);

    } else if (tt_upcase == POptions::T_WEEK) {
      // WEEK
      curl = onel;
      curl.height = time_height;
      curl.minSkipX = minSkipX;
      curl.color = pets_colour(mainplotoptions.timecolour);
      curl.datestyle = DS_WEEK;

      curp = onep;
      curp.type = DATE;
      curp.order = order;
      order += 10;
      curp.layout = curl;

      diaStyle.addPrimitive(curp);

    } else if (tt_upcase == POptions::T_MONTH) {
      // MONTH
      curl = onel;
      curl.height = time_height;
      curl.minSkipX = minSkipX;
      curl.color = pets_colour(mainplotoptions.timecolour);
      curl.datestyle = DS_MONTH;
      curl.asNumber = false;

      curp = onep;
      curp.type = DATE;
      curp.order = order;
      order += 10;
      curp.layout = curl;

      diaStyle.addPrimitive(curp);

    } else if (tt_upcase == POptions::T_MONTHNUM) {
      // MONTHNUM
      curl = onel;
      curl.height = time_height;
      curl.minSkipX = minSkipX;
      curl.color = pets_colour(mainplotoptions.timecolour);
      curl.datestyle = DS_MONTH;
      curl.asNumber = true;

      curp = onep;
      curp.type = DATE;
      curp.order = order;
      order += 10;
      curp.layout = curl;

      diaStyle.addPrimitive(curp);

    } else if (tt_upcase == POptions::T_YEAR) {
      // YEAR
      curl = onel;
      curl.height = time_height;
      curl.minSkipX = minSkipX;
      curl.color = pets_colour(mainplotoptions.timecolour);
      curl.datestyle = DS_YEAR;

      curp = onep;
      curp.type = DATE;
      curp.order = order;
      order += 10;
      curp.layout = curl;

      diaStyle.addPrimitive(curp);

    }
  }

  // XAXIS -----------
  curl = onel;
  curl.color = pets_colour(mainplotoptions.linecolour);//ptColor("RED");
  curl.color2 = pets_colour(mainplotoptions.linecolour);//ptColor("RED");

  curp = onep;
  curp.type = XAXIS;
  curp.order = order;
  order += 10;
  curp.layout = curl;

  diaStyle.addPrimitive(curp);

  // AXIS-GRID -----------
  if (mainplotoptions.drawgrid) {
    curl = onel;
    curl.color = ptColor("GREY25");
    curl.linePattern = DOTTED;
    curl.patternInColour = true;
    curl.useTimes = mainplotoptions.gridTimes;

    curp = onep;
    curp.type = GRIDLINE;
    curp.mother = AXES1;
    curp.layout = curl;

    diaStyle.addPrimitive(curp);
  }

  // The TOP title -----------
  curl = onel;
  curl.text = atitle;
  curl.align = CENTER;
  curl.color = pets_colour(mainplotoptions.textcolour);//ptColor("RED");
  curl.font = HEADER2;
  curl.height = 30;
  curl.spacing = SMALL;

  curp = onep;
  curp.type = TEXT;
  curp.id = ID_UNDEF;
  curp.mother = DIAGRAM;
  curp.plotAll = false;
  curp.order = order;
  order += 10;
  curp.enabled = true;
  curp.layout = curl;

  diaStyle.addPrimitive(curp);

  // set background color for plot
  diaStyle.backgroundColor(pets_colour(mainplotoptions.fillcolour));

  // run through all data-elements - add plotelements for each
  int n = tslist.size();
  for (int i = 0; i < n; i++) {
    // TimeSeries
    if (tslist[i].dataOK()) {
      std::set<miutil::miTime> times = tslist[i].times();
      int nPoints = times.size();
      if (nPoints < 1)
        continue;
      // add timeline
      std::vector<miutil::miTime> vt(times.begin(), times.end());
      int tlindex = petsdata->addTimeLine(vt);

      // fetch data - and make a WeatherParameter
      WeatherParameter wp;
      ParId parid;

      DataList dlist = tslist[i].values();
      plotoptions = tslist[i].plotoptions();

      // Parameter-ID from paramid and stationid
      parid.alias = miutil::from_number(tslist[i].paramid()) + "_" + miutil::from_number(tslist[i].stationid());
      parid.level = tslist[i].level();
      parid.model = "HQC";
      parid.run = 0;
      //parid.submodel;

      // make weatherparameter
      wp.setDims(nPoints, 1); // times/dimensions
      wp.setTimeLineIndex(tlindex);
      wp.setId(parid);
      wp.setPolar(true); // always polar vectors here!
      int ndim = 0;
      for (int j = 0; j < nPoints; j++) {
        ValueList vl = dlist[j].values();
        int nd = vl.size();
        if (j != 0 && nd != ndim) {
          METLIBS_LOG_ERROR("data-dimensions bad for parid:" << parid);
          break;
        }
        ndim = nd;
        for (int ind = 0; ind < ndim; ind++) {
          float value = vl[ind];
          wp.setData(j, ind, value);
        }
      }
      wp.calcAllProperties();
      // add weatherparameter
      petsdata->addParameter(wp);

      // add element to style..
      curp = onep;
      curl = onel;
      fillElement(curp, curl, plotoptions);

      if (plotoptions.plottype == POptions::type_line
          || plotoptions.plottype == POptions::type_histogram)
      {
        // check if axis exists
        if (!usedaxis[curl.axis]) {
          // add Y-axis
          curp.layout.yaid = axisid[curl.axis];
          curp.layout.lineWidth = 2;
          curp.layout.numTickMajor = 10;
          curp.layout.numTickMinor = 2;
          curp.layout.text = plotoptions.axisname;
          curp.layout.horLabels = true;
          curp.layout.horLabelOffset = label_offset++;

          curp.type = YAXIS_STATIC;
          diaStyle.addPrimitive(curp);
          usedaxis[curl.axis] = true;
          // reset for element
          curp = onep;
          curl = onel;
          fillElement(curp, curl, plotoptions);
        }

        // add element...
        curp.layout.yaid = axisid[curl.axis];
        curp.id = parid;
        diaStyle.addPrimitive(curp);

      } else { // outside AXIS -element
        curp.id = parid;
        diaStyle.addPrimitive(curp);
      }
    }
  }

  //   diaStyle.print();
  //   cerr << "===================== PETSDATA ==================" << endl;
  //   cerr << *petsdata << endl;
  //   cerr << "===================== PETSDATA END ==============" << endl;

  return true;
}

bool TSdrawarea::prepareDiagram()
{
  if (!petsdata)
    return (false);
  if (diagram)
    delete diagram;
  diagram = new ptDiagram(&diaStyle);

  if (!diagram->attachData(petsdata))
    return (false);

  ptColor bgColor;
  if (!diagram->makeDefaultPlotElements(&bgColor))
    return false;

  diagram->setViewport(width, height, glwidth, glheight);

  glClearColor(bgColor.colTable[0], bgColor.colTable[1], bgColor.colTable[2],
      1.0);

  // set correct weathersymbols
  miSymbol tmpSymbol;
  PlotElement* pe = 0;
  std::vector<SymbolElement*> sev;
  int minsymb = wsymbols.minCustom(), maxsymb = wsymbols.maxCustom();
  while ((pe = diagram->findElement(SYMBOL, pe))) {
    sev.push_back((SymbolElement*) pe);
    pe = pe->next;
  }
  if (sev.size()) {
    std::vector<miutil::miString> symbimages;
    std::string stmp;
    for (int i = minsymb; i <= maxsymb; i++) {
      tmpSymbol = wsymbols.getSymbol(i);
      //stmp= setup.path.images+tmpSymbol.picture();
      symbimages.push_back(stmp);
    }
    for (size_t i=0; i<sev.size(); i++)
      sev[i]->setImages(minsymb, symbimages);
  }

  return true;
}

void TSdrawarea::setTimemark(const miutil::miTime& nt, const std::string& name)
{
  timemarks[name] = nt;
}

void TSdrawarea::clearTimemarks(const std::string& el)
{
  if (el.empty())
    timemarks.clear();

  if (timemarks.count(el))
    timemarks.erase(el);
}

void TSdrawarea::useTimemarks()
{
  if (timemarks.empty() || !petsdata || !diagram)
    return;

  std::map<std::string, miutil::miTime>::iterator itr = timemarks.begin();
  std::vector<miutil::miTime> mark(1);

  for (; itr != timemarks.end(); itr++) {
    if (PlotElement* pe = diagram->findElement(itr->first, 0)) {
      TimemarkerElement* ptm = (TimemarkerElement*) pe;
      mark[0] = itr->second;
      ptm->setTimes(mark);
    }
  }
}

void TSdrawarea::plot()
{
  if (!diagram)
    return;

  //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClear(GL_COLOR_BUFFER_BIT);

  useTimemarks();
  diagram->plot();

  glFlush();
}
