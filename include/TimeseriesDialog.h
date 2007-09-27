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
#ifndef TIMESERIESDIALOG_H
#define TIMESERIESDIALOG_H

#include <stdlib.h>
#include <iostream>
#include <PlotOptions.h>
//#include <qmainwindow.h>
#include <qdialog.h>
//#include <qprogressdialog.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qdatetime.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qcolor.h>
#include <qstringlist.h>
#include <kvData.h>
#include <kvStation.h>
#include <kvDbGate.h>
#include <vector>

using namespace std;

class miTimeSpinBox;

class TimeseriesDialog : public QDialog {
  Q_OBJECT
public:
  TimeseriesDialog();

  void hideAll();
  void showAll();

  void getResults(vector<miutil::miString>& parameter, 
		  miutil::miTime& fromTime,
		  miutil::miTime& toTime,
		  vector<int>& stationID,
		  vector<POptions::PlotOptions>& plotoptions);

  QCheckBox* obsCheckBox;
  QCheckBox* modCheckBox;

public slots:

  void setFromTimeSlot(const miutil::miTime& t);
  void setToTimeSlot(const miutil::miTime& t);
  void parameterSelectionChanged(QListBoxItem*);
  void stationSelected(QListBoxItem*);
  void resultSelected(QListBoxItem*);
  void deleteSlot();
  void deleteAllSlot();
  void linecolourSlot(int);
  void lineSlot(int);
  void linewidthSlot(int);
  void markerSlot(int);
  void fillcolourSlot(int);
  void newStationList(std::vector<QString>& stationList);
  void newParameterList(const QStringList& parameterList);

private:

  void fillColours();
  std::vector<POptions::Colour> colours;
  std::vector<POptions::Linetype> linetypes;
  std::vector<POptions::Marker> markers;
  std::vector<POptions::yAxis> axes;

  struct tsInfo{
    int parameter;
    int station;
    int linecolour;
    int linetype;
    int linewidth;
    int marker;
    int fillcolour;
  };
  vector<tsInfo> tsinfo;
  int currentResult;
  bool freeze;

  QLabel* parLabel;
  miTimeSpinBox* from;
  miTimeSpinBox* to;

  QPushButton* newcurveButton;

  QListBox* statlb;
  QListBox* parameterListbox;
  QListBox* resultListbox;

  QComboBox* linecolourBox;  
  QComboBox* lineBox;
  QComboBox* linewidthBox;
  QComboBox* markerBox;
  QComboBox* fillcolourBox;

signals:
  void TimeseriesHide();
  void TimeseriesApply();
};
#endif
