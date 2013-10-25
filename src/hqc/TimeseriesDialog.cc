/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2007-2013 met.no

Contact information:
Norwegian Meteorological Institute
Box 43 Blindern
0313 OSLO
NORWAY
email: kvalobs-dev@met.no

This file is part of HQC

HQC is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

HQC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with HQC; if not, write to the Free Software Foundation Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "TimeseriesDialog.h"
#include "util/gui/HideApplyBox.hh"
#include "common/KvMetaDataBuffer.hh"
#include "util/gui/MiDateTimeEdit.hh"
#include "util/gui/qtQTUtil.h"
#include "common/KvHelpers.hh"

#include <QtGui/QLabel>
#include <QtGui/qlayout.h>
#include <Qt3Support/Q3VBoxLayout>
#include <Qt3Support/Q3HBoxLayout>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace miutil;

TimeseriesDialog::TimeseriesDialog(QWidget* parent)
    : QDialog(parent)
{

  setCaption(tr("Time series HQC"));
//Test
  Q3ButtonGroup* dTypes = new Q3ButtonGroup(1,
					  Qt::Horizontal,
					  tr("Data types"), this);
  obsCheckBox = new QCheckBox(tr("Observationer"), dTypes);
  modCheckBox = new QCheckBox(tr("Model data"), dTypes);
  //  connect( obsCheckBox, SIGNAL(
  //  QGridLayout* checkLayout = new QGridLayout(dTypes->layout());
  //  checkLayout->addWidget(obsCheckBox,0,0);
  //  checkLayout->addWidget(modCheckBox,
//Test slutter
  QLabel* parameterLabel = new QLabel(tr("Parameter"),this);
  parameterLabel->setFont(QFont("Arial", 12));
  parameterLabel->setAlignment(Qt::AlignLeft);
  parameterLabel->setPaletteForegroundColor(Qt::darkBlue);

  parameterListbox = new Q3ListBox(this);
  parameterListbox->setMinimumHeight(85);
  connect( parameterListbox, SIGNAL( selectionChanged(Q3ListBoxItem *) ),
	   this, SLOT( parameterSelectionChanged(Q3ListBoxItem * ) ) );

  /////////////////////////////////////////////////////////////////////
  QLabel* statLabel = new QLabel(tr("Station"),this);
  statLabel->setFont(QFont("Arial", 12));
  statLabel->setAlignment(Qt::AlignLeft);
  statLabel->setPaletteForegroundColor(Qt::darkBlue);

  statlb = new Q3ListBox(this);
  statlb->setMinimumHeight(85);
  connect( statlb, SIGNAL( selectionChanged(Q3ListBoxItem *) ),
	   this, SLOT( stationSelected(Q3ListBoxItem * ) ) );
  ////////////////////////////////////////////////////////////////////////
  currentResult=-1;
  QLabel* resultLabel = new QLabel(tr("Chosen time series"),this);
  resultLabel->setFont(QFont("Arial", 12));
  resultLabel->setAlignment(Qt::AlignLeft);
  resultLabel->setPaletteForegroundColor(Qt::darkBlue);
  resultListbox = new Q3ListBox(this);
  resultListbox->setMinimumHeight(85);

  connect( resultListbox, SIGNAL( selectionChanged(Q3ListBoxItem *) ),
	   this, SLOT( resultSelected(Q3ListBoxItem * ) ) );
  /////////////////////////////////////////////////////////////////////////

  QPushButton* delButton = new QPushButton(tr("Delete"),this);
  QPushButton* delallButton = new QPushButton(tr("Delete all"),this);
  newcurveButton = new QPushButton(tr("New curve"),this);
  newcurveButton->setToggleButton(true);

  connect( delButton, SIGNAL(clicked()),SLOT(deleteSlot()));
  connect( delallButton, SIGNAL(clicked()),SLOT(deleteAllSlot()));

  Q3HBoxLayout* delLayout = new Q3HBoxLayout();
  delLayout->addWidget(delButton);
  delLayout->addWidget(delallButton);
  delLayout->addWidget(newcurveButton);
  /////////////////////////////////////////////////////////////////////////
   fillColours();

  POptions::Colour::definedColours(colours);
  int nr_col = colours.size();
  //  cerr <<"COLOURS:"<<colours.size()<<endl;
  QColor* pixcolor = new QColor[nr_col];
  for( int i=0; i<nr_col; i++ ){
    pixcolor[i]=QColor(colours[i].R(),colours[i].G(),colours[i].B());
  }

  QLabel* lineLabel = new QLabel(tr("Line"),this);
  lineLabel->setFont(QFont("Arial", 12));
  lineLabel->setAlignment(Qt::AlignLeft);
  lineLabel->setPaletteForegroundColor(Qt::darkBlue);

  linecolourBox = ComboBox( this, pixcolor, nr_col, true, 0 );

  lineBox = LinetypeBox(this,true,0);

  linewidthBox = new QComboBox(this);
  linewidthBox->insertItem("1");
  linewidthBox->insertItem("2");
  linewidthBox->insertItem("3");
  linewidthBox->insertItem("4");

  connect(linecolourBox, SIGNAL(activated(int)),SLOT(linecolourSlot(int)));
  connect(lineBox, SIGNAL(activated(int)),SLOT(lineSlot(int)));
  connect(linewidthBox, SIGNAL(activated(int)),SLOT(linewidthSlot(int)));

  Q3HBoxLayout* lineLayout = new Q3HBoxLayout();
  lineLayout->addWidget(lineBox,10);
  lineLayout->addWidget(linewidthBox,10);
  lineLayout->addWidget(linecolourBox,10);

  QLabel* markerLabel = new QLabel(tr("Marker"),this);
  markerLabel->setFont(QFont("Arial", 12));
  markerLabel->setAlignment(Qt::AlignLeft);
  markerLabel->setPaletteForegroundColor(Qt::darkBlue);

  markerBox = new QComboBox(this);
  markerBox->insertItem(tr("Circle"));
  markerBox->insertItem(tr("Rectangle"));
  markerBox->insertItem(tr("Triangle"));
  markerBox->insertItem(tr("Diamond"));
  markerBox->insertItem(tr("Star"));
  markerBox->insertItem(tr(""));

  fillcolourBox = ComboBox( this, pixcolor, nr_col, true, 0 );

  connect(markerBox, SIGNAL(activated(int)),SLOT(markerSlot(int)));
  connect(fillcolourBox, SIGNAL(activated(int)),SLOT(fillcolourSlot(int)));

  Q3HBoxLayout* markerLayout = new Q3HBoxLayout();
  markerLayout->addWidget(markerBox,10);
  markerLayout->addWidget(fillcolourBox,10);

// ///////////////////// to from ///////////////////////////////////////////

  QDateTime dt_to = timeutil::nowWithMinutes0Seconds0();
  dt_to = dt_to.addSecs(3600 + 60);
  dte_to = new MiDateTimeEdit(dt_to, this);
  dte_to->setDisplayFormat("yyyy-MM-dd hh:mm");

  QDateTime dt_from = dt_to;
  dt_from = dt_from.addDays(-2);
  dt_from = dt_from.addSecs(3600*(17-dt_from.time().hour()) + 60*45);
  dte_from = new MiDateTimeEdit(dt_from, this);
  dte_from->setDisplayFormat("yyyy-MM-dd hh:mm");

  connect( dte_from, SIGNAL(dateTimeChanged(const QDateTime&)),
           this, SLOT(setMinToTime(const QDateTime&)));

  connect( dte_to, SIGNAL(dateTimeChanged(const QDateTime&)),
           this, SLOT(setMaxFromTime(const QDateTime&)));

  //////////////////// apply & hide ///////////////////////////////////////////
  HideApplyBox* hab = new HideApplyBox(this);
  connect(hab, SIGNAL(hide()) , SLOT(hide()));
  connect(hab, SIGNAL(apply()), SIGNAL(TimeseriesApply()));

  Q3VBoxLayout* topLayout = new Q3VBoxLayout(this,10);
  topLayout->addWidget(dTypes);
  topLayout->addWidget(parameterLabel);
  topLayout->addWidget(parameterListbox);
  topLayout->addWidget(statLabel);
  topLayout->addWidget(statlb);
  topLayout->addWidget(resultLabel);
  topLayout->addWidget(resultListbox);
  topLayout->addLayout(delLayout);
  topLayout->addWidget(lineLabel);
  topLayout->addLayout(lineLayout);
  topLayout->addWidget(markerLabel);
  topLayout->addLayout(markerLayout);
  topLayout->addWidget(new QLabel(tr("From:")));
  topLayout->addWidget(dte_from);
  topLayout->addWidget(new QLabel(tr("To:")));
  topLayout->addWidget(dte_to);
  topLayout->addWidget(hab);

  //Init
  tsInfo ts;
  ts.parameter = parameterListbox->currentItem();
  ts.station = statlb->currentItem();
  ts.linecolour = linecolourBox->currentItem();
  ts.fillcolour = fillcolourBox->currentItem();
  ts.linetype = lineBox->currentItem();
  ts.linewidth = linewidthBox->currentItem();
  ts.marker = markerBox->currentItem();
  tsinfo.push_back(ts);
  freeze=false;
}

void TimeseriesDialog::setMinToTime(const QDateTime& dt)
{
    dte_to->setMinimumDateTime(dt);
}

void TimeseriesDialog::setMaxFromTime(const QDateTime& dt)
{
    dte_from->setMaximumDateTime(dt);
}

void TimeseriesDialog::deleteSlot( ){

  int item =resultListbox->currentItem();
  if(item> -1){
    resultListbox->removeItem(item);
    tsinfo.erase(tsinfo.begin()+item);
  }
  std::cerr <<"delete  ts:"<<tsinfo.size()<<std::endl;
}

void TimeseriesDialog::setFromTimeSlot(const QDateTime& dt)
{
  dte_from->setMinimumDateTime(dt);
  dte_from->setDateTime(dt);
}

void TimeseriesDialog::setToTimeSlot(const QDateTime& dt)
{
  dte_to->setMaximumDateTime(dt);
  dte_to->setDateTime(dt);
}

void TimeseriesDialog::deleteAllSlot( )
{
  resultListbox->clear();
  tsinfo.clear();
  tsInfo ts;
  ts.parameter   = parameterListbox->currentItem();
  ts.station = statlb->currentItem();
  ts.linecolour  = linecolourBox->currentItem();
  ts.linetype    = lineBox->currentItem();
  ts.fillcolour  = fillcolourBox->currentItem();
  ts.linewidth   = linewidthBox->currentItem();
  ts.marker      = markerBox->currentItem();
  tsinfo.push_back(ts);
}

void TimeseriesDialog::linecolourSlot(int){
  if(freeze) return;
  int item =resultListbox->currentItem();
  if(item>-1 ){
    tsinfo[item].linecolour=linecolourBox->currentItem();
  }  else {
    tsinfo[0].linecolour=linecolourBox->currentItem();
  }
}

void TimeseriesDialog::lineSlot(int){
  if(freeze) return;

  int item =resultListbox->currentItem();
  if(item>-1 ){
    tsinfo[item].linetype=lineBox->currentItem();
  }  else {
    tsinfo[0].linetype=lineBox->currentItem();
  }

}

void TimeseriesDialog::linewidthSlot(int){
  if(freeze) return;

  int item =resultListbox->currentItem();
  if(item>-1 ){
    tsinfo[item].linewidth=linewidthBox->currentItem();
  }  else {
    tsinfo[0].linewidth=linewidthBox->currentItem();
  }
}

void TimeseriesDialog::markerSlot(int){
  if(freeze) return;
  int item =resultListbox->currentItem();
  if(item>-1 ){
    tsinfo[item].marker=markerBox->currentItem();
  }  else {
    tsinfo[0].marker=markers[markerBox->currentItem()];
  }
}
void TimeseriesDialog::fillcolourSlot(int){
  if(freeze) return;
  int item =resultListbox->currentItem();
  if(item>-1 ){
    tsinfo[item].fillcolour=fillcolourBox->currentItem();
  }  else {
    tsinfo[0].fillcolour=fillcolourBox->currentItem();
  }
}

void TimeseriesDialog::parameterSelectionChanged(Q3ListBoxItem *item) {
  if(freeze) return;
  if(parameterListbox->currentItem() == -1 ) return;
  if(statlb->currentItem() == -1) return;
  freeze=true;
  std::string str = statlb->currentText().latin1();
  boost::algorithm::trim(str);
  str+= " ";
  str += item->text().latin1();

  tsInfo ts;
  ts.parameter  = parameterListbox->currentItem();
  ts.station    = statlb->currentItem();
  ts.linecolour = linecolourBox->currentItem();
  ts.linetype   = lineBox->currentItem();
  ts.linewidth  = linewidthBox->currentItem();
  ts.fillcolour = fillcolourBox->currentItem();
  ts.marker     = markerBox->currentItem();

  if(resultListbox->currentItem()==-1){
    resultListbox->insertItem(str.c_str());
    resultListbox->setSelected(0,true);
    tsinfo[0] = ts;
  }else{
    if( newcurveButton->isOn() ){
      resultListbox->insertItem(str.c_str());
      tsinfo.push_back(ts);
    } else {
      resultListbox->changeItem(str.c_str(),resultListbox->currentItem());
      tsinfo[resultListbox->currentItem()] = ts;
    }
  }
  freeze=false;
}

void TimeseriesDialog::stationSelected(Q3ListBoxItem*) {
  if(freeze) return;
  if( parameterListbox->currentItem() == -1 ) return;
  freeze=true;
  std::string str = statlb->currentText().latin1();
  boost::algorithm::trim(str);
  str+= " ";
  str += parameterListbox->currentText().latin1();

  tsInfo ts;
  ts.parameter  = parameterListbox->currentItem();
  ts.station    = statlb->currentItem();
  ts.linecolour = linecolourBox->currentItem();
  ts.linetype   = lineBox->currentItem();
  ts.linewidth  = linewidthBox->currentItem();
  ts.fillcolour = fillcolourBox->currentItem();
  ts.marker     = markerBox->currentItem();

  if(resultListbox->currentItem()==-1){
    resultListbox->insertItem(str.c_str());
    resultListbox->setSelected(0,true);
    tsinfo[0] = ts;
  }else{
    if( newcurveButton->isOn() ){
      resultListbox->insertItem(str.c_str());
      tsinfo.push_back(ts);
    } else {
      resultListbox->changeItem(str.c_str(),resultListbox->currentItem());
      tsinfo[resultListbox->currentItem()] = ts;
    }
  }

   freeze=false;
}

void TimeseriesDialog::resultSelected(Q3ListBoxItem*)
{
  if(freeze) return;
  freeze=true;
  int index = resultListbox->currentItem();
  parameterListbox->setSelected(tsinfo[index].parameter,true);
  statlb->setSelected(tsinfo[index].station,true);
  std::cerr <<"Result station:"<<tsinfo[index].station<<std::endl;
  linecolourBox->setCurrentItem(tsinfo[index].linecolour);
  lineBox->setCurrentItem(tsinfo[index].linetype);
  linewidthBox->setCurrentItem(tsinfo[index].linewidth);
  markerBox->setCurrentItem(tsinfo[index].marker);
  fillcolourBox->setCurrentItem(tsinfo[index].fillcolour);
  freeze=false;
}

void TimeseriesDialog::newParameterList(const std::vector<int>& parameters)
{
  QStringList parameterNames;
  BOOST_FOREACH(int pid, parameters) {
    parameterNames << Helpers::paramName(pid);
  }
  parameterListbox->clear();
  parameterListbox->insertStringList(parameterNames);
}

void TimeseriesDialog::newStationList(std::vector<QString>& stationList)
{
  //  cerr<<"ENDRER STASJONSLISTE!!!!!!!!!!!!!!!!!!!"<<endl;
  statlb->clear();
  int n = stationList.size();
  for(int i=0; i<n; i++ ){
    statlb->insertItem(stationList[i]);
  }
}

void TimeseriesDialog::getResults(std::vector<std::string>& parameter,
				  timeutil::ptime& fromTime,
				  timeutil::ptime& toTime,
				  std::vector<int>& stationID,
				  std::vector<POptions::PlotOptions>& plotoptions)
{
  fromTime = timeutil::from_QDateTime(dte_from->dateTime());
  toTime   = timeutil::from_QDateTime(dte_to  ->dateTime());

  int nTypes = obsCheckBox->isChecked() + modCheckBox->isChecked();
  int n = resultListbox->count();
  for ( int j = 0; j < nTypes; j++ ) {
    for(int i=0; i<n; i++){
      parameter.push_back(parameterListbox->text(tsinfo[i].parameter).latin1());
      std::string station = statlb->text(tsinfo[i].station).latin1();
      stationID.push_back(atoi(station.c_str()));
      std::string idString = boost::lexical_cast<std::string>(stationID[i]);

      POptions::PlotOptions po;
      po.name=  "Stasjon:" + idString;
      po.label= parameter[i] + " (" + idString + ")";
      po.linecolour = colours[tsinfo[i].linecolour];
      po.linetype   = linetypes[tsinfo[i].linetype];
      po.linewidth  = tsinfo[i].linewidth;
      po.marker     = markers[tsinfo[i].marker];
      po.fillcolour = colours[tsinfo[i].fillcolour];
      po.axis= axes[i];
      plotoptions.push_back(po);

      if (parameter[i] == "DD") {
	plotoptions[i].plottype= POptions::type_vector;
	plotoptions[i].linewidth= 1;
      }
      else if (parameter[i] == "PO" || parameter[i] == "PR"){
	plotoptions[i].axis= axes[0];
	plotoptions[i].axisname= "Trykk";
      }
      else if (parameter[i].find("T") != std::string::npos){
	plotoptions[i].axis= axes[1];
	plotoptions[i].axisname= "Temp.";
      }
      else if (parameter[i].find("RR") != std::string::npos){
	po.plottype=POptions::type_histogram;
	plotoptions[i].axisname= "";
      }
    }
  }
}

void TimeseriesDialog::fillColours()
{
  POptions::Colour::define("black",0,0,0);
  POptions::Colour::define("white",255,255,255);
  POptions::Colour::define("red",255,0,0);
  POptions::Colour::define("green",0,255,0);
  POptions::Colour::define("blue",0,0,255);
  POptions::Colour::define("cyan",0,255,255);
  POptions::Colour::define("magenta",255,0,255);
  POptions::Colour::define("yellow",255,255,0);
  POptions::Colour::define("grey25",64,64,64);
  POptions::Colour::define("grey40",102,102,102);
  POptions::Colour::define("grey45",115,115,115);
  POptions::Colour::define("grey50",128,128,128);
  POptions::Colour::define("grey55",140,140,140);
  POptions::Colour::define("grey60",153,153,153);
  POptions::Colour::define("grey65",166,166,166);
  POptions::Colour::define("grey70",179,179,179);
  POptions::Colour::define("grey75",192,192,192);
  POptions::Colour::define("grey80",204,204,204);
  POptions::Colour::define("grey85",217,217,217);
  POptions::Colour::define("grey90",230,230,230);
  POptions::Colour::define("grey95",243,243,243);
  POptions::Colour::define("mist_red",240,220,220);
  POptions::Colour::define("mist_green",220,240,220);
  POptions::Colour::define("mist_blue",220,240,240);
  POptions::Colour::define("dark_green",0,128,128);
  POptions::Colour::define("brown",179,36,0);
  POptions::Colour::define("orange",255,90,0);
  POptions::Colour::define("purple",90,0,90);
  POptions::Colour::define("light_blue",36,36,255);
  POptions::Colour::define("dark_yellow",179,179,0);
  POptions::Colour::define("dark_red",128,0,0);
  POptions::Colour::define("dark_blue",0,0,128);
  POptions::Colour::define("dark_cyan",0,128,128);
  POptions::Colour::define("dark_magenta",128,0,128);
  POptions::Colour::define("midnight_blue",26,26,110);
  POptions::Colour::define("dnmi_green",44,120,36);
  POptions::Colour::define("dnmi_blue",0,54,125);

  POptions::Linetype::define("full",0xFFFF,1);
  POptions::Linetype::define("dashed",0x00FF,1);
  POptions::Linetype::define("dashdotted",0x0C0F,1);
  POptions::Linetype::define("dashdashdotted",0x1C47,1);
  POptions::Linetype::define("dotted",0xAAAA,2);

  linetypes.push_back(POptions::Linetype("full"));
  linetypes.push_back(POptions::Linetype("dashed"));
  linetypes.push_back(POptions::Linetype("dotted"));
  linetypes.push_back(POptions::Linetype("dashdotted"));
  linetypes.push_back(POptions::Linetype("dashdashdotted"));

  markers.push_back(POptions::M_CIRCLE);
  markers.push_back(POptions::M_RECTANGLE);
  markers.push_back(POptions::M_TRIANGLE);
  markers.push_back(POptions::M_DIAMOND);
  markers.push_back(POptions::M_STAR);

  axes.push_back(POptions::axis_left_left);
  axes.push_back(POptions::axis_right_right);
  axes.push_back(POptions::axis_left_right);
  axes.push_back(POptions::axis_right_left);
  axes.push_back(POptions::axis_right_left);
  axes.push_back(POptions::axis_right_left);
}
