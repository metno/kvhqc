/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id$

Copyright (C) 2007 met.no

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
#include <TimeseriesDialog.h>
#include <qlayout.h>
#include <qtQTUtil.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3VBoxLayout>
#include <miTimeSpinBox.h>

TimeseriesDialog::TimeseriesDialog() : QDialog(0, 0, FALSE) {  

  setCaption("Tidsserie HQC");
  //  setGeometry(10,10,500,500);

//   parlv = new QListView(this);
//   //  parlv->setGeometry(10,10,40,100);
//   //  parlv->setSorting(1);
//   parlv->addColumn("Parameter");
//   parlv->setRootIsDecorated( TRUE );
//   //  parlv->setSelectionMode(QListView::Extended);
//   //  parlv->setAllColumnsShowFocus(TRUE);


//   QListViewItem* airplvi = new QListViewItem(parlv, "Lufttrykk");
//   airplvi->setSelectable(false);
//   QListViewItem* templvi = new QListViewItem(parlv, "Temperatur");
//   templvi->setSelectable(false);
//   QListViewItem* windlvi = new QListViewItem(parlv, "Vind");
//   windlvi->setSelectable(false);
//   QListViewItem* sub1, *sub2, *sub3, *sub4;
//   airplvi->setOpen(TRUE);
//   sub1 = new QListViewItem( airplvi, "PO" );
//   sub2 = new QListViewItem( airplvi, "PR" );
//   sub3 = new QListViewItem( airplvi, "AA" );
//   sub4 = new QListViewItem( airplvi, "PP" );
//   //  parlvi = new QListViewItem(parlv, "Temperatur");
//   templvi->setOpen(TRUE);
//   (void)new QListViewItem( templvi, "TA" );
//   (void)new QListViewItem( templvi, "TAN_12" );
//   (void)new QListViewItem( templvi, "TAX_12" );
//   //  parlvi = new QListViewItem(parlv, "Vind");
//   windlvi->setOpen(TRUE);
//   (void)new QListViewItem( windlvi, "FF" );
//   (void)new QListViewItem( windlvi, "DD" );
//   (void)new QListViewItem( windlvi, "FF_02" );
//   (void)new QListViewItem( windlvi, "DD_02" );
//   (void)new QListViewItem( windlvi, "FG" );
//   (void)new QListViewItem( windlvi, "DG" );
// //   connect( parlv, SIGNAL( selectionChanged( ) ),
// // 	   this, SLOT( parameterSelectionChanged( ) ) );
//   connect( parlv, SIGNAL( selectionChanged( QListViewItem*) ),
// 	   this, SLOT( parameterSelectionChanged(QListViewItem* ) ) );
//Test
  Q3ButtonGroup* dTypes = new Q3ButtonGroup(1, 
					  Qt::Horizontal, 
					  "Datatyper", this);
  obsCheckBox = new QCheckBox("Observasjoner", dTypes);
  modCheckBox = new QCheckBox("Modelldata   ", dTypes);
  //  connect( obsCheckBox, SIGNAL(  
  //  QGridLayout* checkLayout = new QGridLayout(dTypes->layout());
  //  checkLayout->addWidget(obsCheckBox,0,0);
  //  checkLayout->addWidget(modCheckBox,
//Test slutter
  QLabel* parameterLabel = new QLabel("Parameter",this);
  parameterLabel->setFont(QFont("Arial", 12));
  parameterLabel->setAlignment(Qt::AlignLeft);
  parameterLabel->setPaletteForegroundColor(Qt::darkBlue);

  parameterListbox = new Q3ListBox(this);
  parameterListbox->setMinimumHeight(85);
  connect( parameterListbox, SIGNAL( selectionChanged(Q3ListBoxItem *) ),
	   this, SLOT( parameterSelectionChanged(Q3ListBoxItem * ) ) );

  /////////////////////////////////////////////////////////////////////
  QLabel* statLabel = new QLabel("Stasjon",this);
  statLabel->setFont(QFont("Arial", 12));
  statLabel->setAlignment(Qt::AlignLeft);
  statLabel->setPaletteForegroundColor(Qt::darkBlue);

  statlb = new Q3ListBox(this);
  statlb->setMinimumHeight(85);
  connect( statlb, SIGNAL( selectionChanged(Q3ListBoxItem *) ),
	   this, SLOT( stationSelected(Q3ListBoxItem * ) ) );
  //  statlb->setGeometry(10,80,40,100);
//   std::list<kvalobs::kvStation>::const_iterator it=slist.begin();
//   QString stnr;
//   const char* cName;
//   QString name;
//   QString statId;
//   for(;it!=slist.end(); it++){
//     stnr = stnr.setNum(it->stationID());
//     cName = (it->name()).c_str();
//     name = QString(cName);
//     statId = stnr + " " + name;
//     cerr << statId << endl;
//     statlb->insertItem(statId);
//   }

  ////////////////////////////////////////////////////////////////////////
  currentResult=-1;
  QLabel* resultLabel = new QLabel("Valgte Tidsserier",this);
  resultLabel->setFont(QFont("Arial", 12));
  resultLabel->setAlignment(Qt::AlignLeft);
  resultLabel->setPaletteForegroundColor(Qt::darkBlue);
  resultListbox = new Q3ListBox(this);
  resultListbox->setMinimumHeight(85);

  connect( resultListbox, SIGNAL( selectionChanged(Q3ListBoxItem *) ),
	   this, SLOT( resultSelected(Q3ListBoxItem * ) ) );
  /////////////////////////////////////////////////////////////////////////

  QPushButton* delButton = new QPushButton("Slett",this);
  QPushButton* delallButton = new QPushButton("Slett alt",this);
  newcurveButton = new QPushButton("Ny kurve",this);
  newcurveButton->setToggleButton(true);
  //  newCurve = false;

  connect( delButton, SIGNAL(clicked()),SLOT(deleteSlot()));
  connect( delallButton, SIGNAL(clicked()),SLOT(deleteAllSlot()));
  //  connect( newcurveButton, SIGNAL(toggled(bool)),SLOT(newcurveSlot(bool)));

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

  QLabel* lineLabel = new QLabel("Linje",this);
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

  QLabel* markerLabel = new QLabel("Markør",this);
  markerLabel->setFont(QFont("Arial", 12));
  markerLabel->setAlignment(Qt::AlignLeft);
  markerLabel->setPaletteForegroundColor(Qt::darkBlue);

  markerBox = new QComboBox(this);
  markerBox->insertItem("Circle");
  markerBox->insertItem("Rectangle");
  markerBox->insertItem("Triangle");
  markerBox->insertItem("Diamond");
  markerBox->insertItem("Star");
  markerBox->insertItem("");

  fillcolourBox = ComboBox( this, pixcolor, nr_col, true, 0 );

  connect(markerBox, SIGNAL(activated(int)),SLOT(markerSlot(int)));
  connect(fillcolourBox, SIGNAL(activated(int)),SLOT(fillcolourSlot(int)));

  Q3HBoxLayout* markerLayout = new Q3HBoxLayout();
  markerLayout->addWidget(markerBox,10);
  markerLayout->addWidget(fillcolourBox,10);
  
// ///////////////////// to from ///////////////////////////////////////////
    
  from = new miTimeSpinBox ("from",this, "Fra:");
  to   = new miTimeSpinBox ("to",this, "Til:");
  miutil::miTime t(to->time());
  //set minutes to 0
  if( t.min() != 0 ){
    t.addMin(-1*t.min());
    t.addHour(1);
    to->setTime(t);
  }
  t.addDay(-2);
  t.addHour(17-t.hour());
  t.addMin(45-t.min());
  from->setTime(t);
  
  connect( from, SIGNAL(valueChanged(const miutil::miTime&)),
	   to,   SLOT(  setMin(const miutil::miTime&)     ));
  
  connect( to,  SIGNAL(valueChanged(const miutil::miTime&)),
	   from,SLOT(  setMax(const miutil::miTime&)     ));

//   QLabel* fromLabel = new QLabel(this);
//   fromLabel->setText("Fra       ");
//   fromLabel->setFont(QFont("Arial", 12));
//   fromLabel->setAlignment(AlignLeft);
//   fromLabel->setPaletteForegroundColor(darkBlue);
  
//   QLabel* toLabel = new QLabel(this);
//   toLabel->setText("Til       ");
//   toLabel->setFont(QFont("Arial", 12));
//   toLabel->setAlignment(AlignLeft);
//   toLabel->setPaletteForegroundColor(darkBlue);

//   QGridLayout* tofromLayout   = new QGridLayout(2, 2);
//   tofromLayout->addWidget(fromLabel,0,0);
//   tofromLayout->addWidget(fromEdit, 0,1);
//   tofromLayout->addWidget(toLabel,  1,0);
//   tofromLayout->addWidget(toEdit,   1,1);


  //////////////////// apply & hide ///////////////////////////////////////////
  QPushButton* hideButton = new QPushButton("Skjul", this);
  hideButton->setFont(QFont("Arial", 9));

  QPushButton* applyButton = new QPushButton("Utfør", this);
  applyButton->setFont(QFont("Arial", 9));

  QPushButton* hideapplyButton = new QPushButton("Utfør+Skjul", this);
  hideapplyButton->setFont(QFont("Arial", 9));


  Q3HBoxLayout* buttonLayout = new Q3HBoxLayout();
  buttonLayout->addWidget(hideButton,10);
  buttonLayout->addWidget(applyButton,10);
  buttonLayout->addWidget(hideapplyButton,10);

  connect(hideButton,      SIGNAL(clicked()), SIGNAL( TimeseriesHide()));
  connect(hideapplyButton, SIGNAL(clicked()), SIGNAL( TimeseriesHide()));
  connect(hideapplyButton, SIGNAL(clicked()), SIGNAL( TimeseriesApply()));
  connect(applyButton,     SIGNAL(clicked()), SIGNAL( TimeseriesApply()));

  Q3VBoxLayout* topLayout = new Q3VBoxLayout(this,10);
  //  topLayout->addWidget(parlv);
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
  //   topLayout->addLayout(tofromLayout);
  topLayout->addWidget(from);
  topLayout->addWidget(to);
  topLayout->addLayout(buttonLayout);

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

void TimeseriesDialog::deleteSlot( ){

  int item =resultListbox->currentItem();
  if(item> -1){
    resultListbox->removeItem(item); 
    tsinfo.erase(tsinfo.begin()+item);
  }
  cerr <<"delete  ts:"<<tsinfo.size()<<endl;
}

void TimeseriesDialog::setFromTimeSlot(const miutil::miTime& t)
{
  from->setMin(t);
  from->setTime(t);
//   cerr <<"From time:"<<t.isoTime()<<endl;
}

void TimeseriesDialog::setToTimeSlot(const miutil::miTime& t)
{
  to->setMax(t);
  to->setTime(t);
//   cerr <<"To time:"<<t.isoTime()<<endl;
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
  //  cerr <<"deleteAll  ts:"<<tsinfo.size()<<endl;
}

// void TimeseriesDialog::newcurveSlot( bool on ){

//   newCurve = on;
//   int item =resultListbox->currentItem();
//   if(item>-1 ){
//     resultListbox->insertItem(resultListbox->text(item),item); 
//     tsInfo ts = tsinfo[item];
//     tsinfo.push_back(ts);
//   }

//}    

void TimeseriesDialog::linecolourSlot( int i){
  if(freeze) return;
  int item =resultListbox->currentItem();
  if(item>-1 ){
    tsinfo[item].linecolour=linecolourBox->currentItem();
  }  else {
    tsinfo[0].linecolour=linecolourBox->currentItem();
  }
}

void TimeseriesDialog::lineSlot(int i ){
  if(freeze) return;

  int item =resultListbox->currentItem();
  if(item>-1 ){
    tsinfo[item].linetype=lineBox->currentItem();
  }  else {
    tsinfo[0].linetype=lineBox->currentItem();
  }

}

void TimeseriesDialog::linewidthSlot(int i ){
  if(freeze) return;

  int item =resultListbox->currentItem();
  if(item>-1 ){
    tsinfo[item].linewidth=linewidthBox->currentItem();
  }  else {
    tsinfo[0].linewidth=linewidthBox->currentItem();
  }

}

void TimeseriesDialog::markerSlot( int i){
  if(freeze) return;
  int item =resultListbox->currentItem();
  if(item>-1 ){
    tsinfo[item].marker=markerBox->currentItem();
  }  else {
    tsinfo[0].marker=markers[markerBox->currentItem()];
  }
}
void TimeseriesDialog::fillcolourSlot( int i){
  if(freeze) return;
  int item =resultListbox->currentItem();
  if(item>-1 ){
    tsinfo[item].fillcolour=fillcolourBox->currentItem();
  }  else {
    tsinfo[0].fillcolour=fillcolourBox->currentItem();
  }
}

void TimeseriesDialog::showAll(){
  this->show();
}


void TimeseriesDialog::hideAll(){
  this->hide();
}


// QString TimeseriesDialog::getStation() {
//   //    return plotStat->text();
//     return statlb->currentText();
// } 

// vector<QString> TimeseriesDialog::getParam() {
//   vector<QString> parName;
//   parName.push_back(parlv->currentItem()->text(0).latin1());
//   return parName;
// } 


// int TimeseriesDialog::getnumts() {
  
//   return tsinfo.size();

// } 

// QString TimeseriesDialog::getStation(int i) {
  
//   if(tsinfo.size()>i)
//     return statlb->text(tsinfo[i].station);

//   QString s;
//   return s;
// }

// QString TimeseriesDialog::getParam(int i) {
  
//   if(tsinfo.size()>i)
//     return tsinfo[i].parameter->text(0);

//   QString s;
//   return s;
// }

// POptions::PlotOptions TimeseriesDialog::getPlotOptions(int i) {
//   POptions::PlotOptions popt;

//   if(tsinfo.size()>i){
//     popt.linecolour = linecolours[tsinfo[i].colour];
//     popt.linetype = linetypes[tsinfo[i].linetype];
//     popt.linewidth = tsinfo[i].linewidth;
//     popt.marker = markers[tsinfo[i].marker];
//     popt.fillcolour = colours[tsinfo[i].fillcolour];
// }

//   return popt;
// }

void TimeseriesDialog::parameterSelectionChanged(Q3ListBoxItem *item) {
  if(freeze) return;
  //  cerr << "Current item is " << item->text(0) << endl;
  //  if(item->parent() == 0) return;
  if(parameterListbox->currentItem() == -1 ) return;
  if(statlb->currentItem() == -1) return;
  freeze=true;
  miString str = statlb->currentText().latin1();
  str.trim(); 
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
    resultListbox->insertItem(str.cStr());
    resultListbox->setSelected(0,true);
    tsinfo[0] = ts;
  }else{
    if( newcurveButton->isOn() ){
      resultListbox->insertItem(str.cStr());
      tsinfo.push_back(ts);
    } else {
      resultListbox->changeItem(str.cStr(),resultListbox->currentItem());
      tsinfo[resultListbox->currentItem()] = ts;
    }
  }
  freeze=false;
} 


void TimeseriesDialog::stationSelected(Q3ListBoxItem * item) {
   if(freeze) return;
   //   cerr << "Station selected: " << station << endl;
//   //  plotStat = item;
  if( parameterListbox->currentItem() == -1 ) return;
  //  if(parameterListbox->currentItem()->parent() == 0) return;
   freeze=true;
  miString str = statlb->currentText().latin1();
  str.trim(); 
  str+= " "; 
  //  str += parameterListbox->currentItem()->text(0).latin1();
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
    resultListbox->insertItem(str.cStr());
    resultListbox->setSelected(0,true);
    tsinfo[0] = ts;
  }else{
    if( newcurveButton->isOn() ){
      resultListbox->insertItem(str.cStr());
      tsinfo.push_back(ts);
    } else {
      resultListbox->changeItem(str.cStr(),resultListbox->currentItem());
      tsinfo[resultListbox->currentItem()] = ts;
    }
  }

   freeze=false;
} 

void TimeseriesDialog::resultSelected(Q3ListBoxItem * item) 
{
  if(freeze) return;
  //cerr <<"Result   tsinfo.size:"<<tsinfo.size()<<endl;
  freeze=true;
  int index = resultListbox->currentItem();
  //cerr <<"Result   index:"<<index<<endl;
  parameterListbox->setSelected(tsinfo[index].parameter,true);
  statlb->setSelected(tsinfo[index].station,true);
  cerr <<"Result station:"<<tsinfo[index].station<<endl;
  linecolourBox->setCurrentItem(tsinfo[index].linecolour);
  lineBox->setCurrentItem(tsinfo[index].linetype);
  linewidthBox->setCurrentItem(tsinfo[index].linewidth);
  markerBox->setCurrentItem(tsinfo[index].marker);
  fillcolourBox->setCurrentItem(tsinfo[index].fillcolour);
  //  currentResult = index;
    freeze=false;
}

void TimeseriesDialog::newParameterList(const QStringList& parameterList)
{
  parameterListbox->clear();
  parameterListbox->insertStringList(parameterList);
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

void TimeseriesDialog::getResults(vector<miString>& parameter, 
				  miutil::miTime& fromTime,
				  miutil::miTime& toTime,
				  vector<int>& stationID,
				  vector<POptions::PlotOptions>& plotoptions)
{
  fromTime = from->time();
  toTime   = to->time();

  int nTypes = obsCheckBox->isChecked() + modCheckBox->isChecked();  
  int n = resultListbox->count();
  for ( int j = 0; j < nTypes; j++ ) {
    for(int i=0; i<n; i++){
      parameter.push_back(parameterListbox->text(tsinfo[i].parameter).latin1());
      miString station = statlb->text(tsinfo[i].station).latin1();
      stationID.push_back(atoi(station.cStr()));
      miString idString(stationID[i]);
      
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
	//plotoptions.label= miString(parameter[i].latin1());
      }
      else if (parameter[i] == "PO" || parameter[i] == "PR"){
	plotoptions[i].axis= axes[0];
	plotoptions[i].axisname= "Trykk";
      }
      else if (parameter[i].contains("T")){
	plotoptions[i].axis= axes[1];
	plotoptions[i].axisname= "Temp.";
      }
      else if (parameter[i].contains("RR")){
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
//   initColours("white",        255,255,255);
//   initColours("grayWhite",    224,224,224);
//   initColours("lightGray",    192,192,192);
//   initColours("gray",         160,160,164);
//   initColours("darkGray",     128,128,128);
//   initColours("black",        0,0,0);
  
//   initColours("blue",         0,0,255);
//   initColours("red",          255,0,0);
//   initColours("green",        0,255,0);
//   initColours("cyan",         0,255,255);
//   initColours("magenta",      255,0,255);
//   initColours("yellow",       255,255,0);
  
//   initColours("lightBlue",    51,51,255);
  
//   initColours("darkRed",      128,0,0);
//   initColours("darkGreen",    0,128,0);
//   initColours("darkBlue",     0,0,128);
//   initColours("darkCyan",     0,128,128);
//   initColours("darkMagenta",  128,0,128);
//   initColours("darkYellow",   128,128,0);
  
//   initColours("brown",        178,51,0);
//   initColours("orange",       255,89,0);
//   initColours("purple",       160,32,240);
  
//   initColours("midnightBlue", 25,25,112);
//   initColours("dnmiGreen",    43,120,36);
//   initColours("dnmiBlue",     0,54,125);
  
//   initColours("green2",       0,238,0);
//   initColours("green3",       0,205,0);
//   initColours("green4",       0,139,0);
  
//   initColours("flesh",      240,158,92);
//   initColours("seablue",    117,199,242);
  
//   initColours("landgul",    255,240,196);
//   initColours("havblå",     225,255,255);
//   initColours("gulbrun",    255,164,71);
 }

// void TimeseriesDialog::initColours(miString name, int r, int g, int b)
// {

//   ColourInfo ci;
//   ci.name=name;
//   ci.rgb[0] = r;
//   ci.rgb[1] = g;
//   ci.rgb[2] = b;

//   colours.push_back(ci);

// }
