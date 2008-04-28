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
#ifndef DATATABLE_H
#define DATATABLE_H

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <qtable.h>
#include <qstring.h>
#include <qpainter.h>
#include <puTools/miTime>
#include <puTools/miString>
#include <math.h>
#include "hqcdefs.h"
#include <kvalobs/kvData.h>
#include <qvalidator.h>

//#include "typeinfotooltip.h"
#include <FailDialog.h>

/**
 * \brief The data table
 *
 * Produces a table which shows the original and corrected values
 * for selected parameters within the selected time interval.  The user can
 * change the corrected values of the parameters. 
 */

class DataTooltipHandler;

class DataTable : public QTable {
  Q_OBJECT
public:
  DataTable(QStringList, 
	    int,
	    int,
	    int*, 
	    int, 
	    QWidget*, 
	    listType, 
	    mettType, 
	    //	   QString, 
	    int, 
	    int,
	    bool);
  ~DataTable(); 
  void sortColumn( int col, bool ascending, bool wholeRows );
  void swapRows( int row1, int row2, bool swapHeader );
  mettType mt() {return mety;}

  /**
   * \brief Get a kvData object for the current cell.
   *
   * \warning Changes made by the user will not be reflected in the object.
   *
   * \return The relevant kvData object, or an empty kvData object on failure.
   */
  kvalobs::kvData getKvData() const;

  /**
   * \brief Get a kvData object for the cell at (row, col).
   *
   * \warning Changes made by the user will not be reflected in the object.
   *
   * \return The relevant kvData object, or an empty kvData object on failure.
   */
  kvalobs::kvData getKvData( int row, int col ) const;
  
  /**
   * \brief Get the original index in hqcmainwindow::datalist, based on the row
   *        in the list
   */ 
  int originalIndex( int row ) const;
  
private:
  //DataTooltipHandler *efh;
  //TypeInfoToolTip* tyTT;
  int parNo[NOPARAMALL];

  /**
   * \return True if the given parameter has model values.
   */
  bool paramHasModel(int);
  /**
   * \return 0 if the given parameter is a code, 1 otherwise.
   */
  int paramIsCode(int);
  bool timeSort;
  mettType mety;
  vector<int> originalIndexes;

public slots:
  /**
   * \brief Move cursor to the given station for the given obstime.
   */
  void selectStation( int station, const miutil::miTime & obstime );

private slots:
  void focusTable(QString&);
  void toggleSort();

  //signals:
  //  void statTimeReceived(QString&);

};


class TableItem : public QTableItem{
public:
  TableItem( QTable *t, 
	     EditType et, 
	     const QString &txt ) : QTableItem( t, et, txt ) {}
  QString key() const;

  void paint(QPainter*, const QColorGroup&, const QRect&, bool);

  bool isModelVal;
};

#endif
