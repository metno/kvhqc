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


#error This file should not be used

#ifndef TABWINDOW_H
#define TABWINDOW_H

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <qstring.h>
#include <puTools/miTime>
#include <puTools/miString>
#include "hqcdefs.h"
#include "hqcmain.h"

class HqcMainWindow;

class MDITabWindow: public QWidget
{
  friend class HqcMainWindow;
    Q_OBJECT
public:
    MDITabWindow( QWidget*, 
		  const char*, 
		  int,const miutil::miTime&, 
		  const miutil::miTime&, 
		  miutil::miTime&, 
		  miutil::miTime&, 
		  listType, 
		  listType, 
		  mettType, 
		  QString&, 
		  QStringList&,
		  int,
		  int*,
		  vector<model::KvalobsData>&,
		  vector<modDatl>&, 
		  list<kvStation>&,
		  int, 
		  int,
		  bool,
		  QString& );
    ~MDITabWindow();

private:
    void showDianaAnalysis(int);
    void showSelectedParam(int, int);
    void showChangedValue(int, int, QString);
    void readLimits();
    bool legalTime(int, int, double);
    bool legalValue(double, int);
    QString dianaName(QString);
    QMap<int, float> lowMap;
    QMap<int, float> highMap;
    //    std::list<kvalobs::kvObsPgm> obsPgmList;
    //    std::list<long> statList;

private slots:
    void tableCellClicked(int, int, int, const QPoint&);
    void tableCellClicked(int, int);
    void tableValueChanged(int, int);
    void updateKvBase(int, int);
    //    void focusTable(QString&);
signals:
//    void message(const QString&, int );

private:
    //    ErrorHead* erHead;
    void readErrorsFromqaBase(int&, 
			      int&, 
			      const miutil::miTime&, 
			      const miutil::miTime&, 
			      miutil::miTime&, 
			      miutil::miTime&, 
			      listType, 
			      listType, 
			      QString);
    int numErr;
    int noParam;
};

#endif
