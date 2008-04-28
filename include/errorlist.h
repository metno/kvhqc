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
#ifndef ERRORLIST_H
#define ERRORLIST_H

#include <set>

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

#include "StationInfoToolTip.h"
#include <FailDialog.h>

class ErrorListFirstCol;

using namespace std;
using namespace kvalobs;

class HqcMainWindow;
class ExtendedFunctionalityHandler;

const float rejectedValue_ = -32767.0;
const float discardedValue_ = -32766.0;
const int npnc = 105;
const int npcc = 27;
const int  parNoControl[] = {  2,  3,  4,  5,  6,  9, 10, 11, 12, 13, 
			       17, 20, 21, 22, 23, 24, 25, 26, 27, 28,
			       44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
			       54, 55, 56, 57,101,102,103,115,116,124,
			       138,191,192,193,194,195,196,197,198,199,
			       202,226,227,229,230,231,232,233,234,235,
			       236,237,238,239,240,241,247,261,271,272,
			       274,275,276,277,278,279,280,281,282,283,
			       284,285,286,287,288,289,290,291,292,293,
			       294,295,296,297,298,299,300,301,302,303,
			       304,305,306,307,308};

const QString controlNoControl[] = {"QC1-2-96:1","QC1-2-97:1","QC1-2-100:1",
				    "QC1-2-101:1","QC1-2-105:1","QC1-2-123A:1",
				    "QC1-2-142:1","QC1-2-143:1","QC1-2-144:1",
				    "QC1-2-145:1","QC1-2-146:1","QC1-2-147:1",
				    "QC1-2-148:1","QC1-2-149:1","QC1-2-150:1",
				    "QC1-2-151:1","QC1-2-152:1","QC1-2-153:1",
				    "QC1-2-154:1","QC1-2-155:1","QC1-2-156:1",
				    "QC1-2-158:1","QC1-2-159:1","QC1-2-160:1",
				    "QC1-2-161:1","QC1-2-162:1","QC1-2-21:1"};

class FlTableItem : public QTableItem {
public:
  FlTableItem(QTable* t, 
	      EditType et, 
	      const QString &txt ) : QTableItem( t, et, txt ) {}
  void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
};

class DkTableItem : public QTableItem {
public:
  DkTableItem(QTable* t, 
	      EditType et, 
	      const QString &txt ) : QTableItem( t, et, txt ) {}
  void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
};

class UsTableItem : public QTableItem {
public:
  UsTableItem(QTable* t, 
	      EditType et, 
	      const QString &txt ) : QTableItem( t, et, txt ) {}
  void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
};

class CrTableItem : public QTableItem {
  bool numbers;
public:
  CrTableItem(QTable* t, 
	      EditType et, 
	      const QString &txt,
	      bool acceptNumbers) 
    : QTableItem( t, et, txt )
    , numbers(acceptNumbers) {}
  void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
  
  virtual QWidget *createEditor() const;
  static const QRegExpValidator validator;
  static const QRegExp re;
};

class OkTableItem : public QCheckTableItem {
public:
  OkTableItem(QTable* t, 
	      const QString &txt ) : QCheckTableItem( t, txt ) {}
  void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
};

class DataCell : public QTableItem {
public:
  DataCell(QTable* t, 
	      EditType et, 
	      const QString &txt ) : QTableItem( t, et, txt ) {
  }
  void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
  //  void setBkgColor(QColor cr) { m_crBkg = cr; }
  //  QColor bkgColor() { return m_crBkg; }
  //private:
  //  QColor m_crBkg;

};
  
class ErrorHead : public QTable {
Q_OBJECT
public:
 ErrorHead(const miutil::miTime&, 
	   const miutil::miTime&, 
	   QWidget*, 
	   int,
	   QString);
 ~ErrorHead();
};

/**
 * \brief The error list
 */

class ErrorList : public QTable {
  Q_OBJECT
public:
  ErrorList(QStringList&,
	    const miutil::miTime&, 
	    const miutil::miTime&,
	    int, 
	    int, 
	    QWidget*, 
	    int, 
	    int, 
	    int*,
	    vector<datl>&, 
	    vector<modDatl>&, 
	    list<kvStation>&,
	    int, 
	    int,
	    QString&);
  ~ErrorList();
  void readLimits();
  struct mem {
    double orig;
    double corr;
    double morig;
    string controlinfo;
    string useinfo;
    string cfailed;
    int flg;
    int sen;
    int lev;
    QString flTyp;
    int parNo;
    QString parName;
    int stnr;
    QString name;
    miutil::miTime obstime;
    miutil::miTime tbtime;
    int typeId;
  };
  /*!
   * \brief 
   */
  struct missObs {
    miutil::miTime oTime;
    int time;
    int parno;
    int statno;
    int missNo;
  };
  vector<missObs> mList;
  vector<mem> missList;

  /**
   * \brief Prompt user to save data.
   * \returns true if user did not press cancel.
   */
  bool maybeSave();


signals:
  /**
   * \brief Reports the selection of a new station and/or obstime in the 
   *        errorlist.
   */
  void stationSelected( int station, const miutil::miTime & obstime );

protected:
//   virtual void hideEvent( QHideEvent * e );
//   virtual void closeEvent( QCloseEvent *e );
//  virtual bool event( QEvent * e );

private:

  struct refs {
    int stnr;
    int rstnr;
    int parNo;
    double dist;
  };
  QString opName;
  StationInfoToolTip* stTT;
  FailInfo::FailDialog* fDlg;
  std::list<kvalobs::kvObsPgm> obsPgmList;
  std::list<long> statList;
  vector<mem> memStore1;
  vector<mem> memStore2;
  vector<mem> memStore3;
private:
  HqcMainWindow * mainWindow;
  /**
   * \brief Indexes of elements wchich are not transferred to the error list 
   */
  vector<int> noError;
  /**
   * \brief Indexes of elements wchich are transferred to the error list 
   */
  vector<int> error;
  /**
   * \brief Decide if an observation is going to the error list or not
   * \return The largest flag value from the automatic control, negative 
   *         if no HQC control is indicated
   */
  int errorFilter(int, string, string, QString&);
  /**
   * \brief Decide if given parameter is to be controlled in HQC
   * \return TRUE if the parameter is to be controlled. 
   */
  bool priorityParameterFilter(int);
  /**
   * \brief Decide if the given control is to be checked in HQC
   * \return 0 if only one control flag is set, and this is not
   *         to be checked in HQC  
   */
  int priorityControlFilter(QString);
  /**
   * \brief Find which observations shall be moved from memory store 1 to error list
   */
  void checkFirstMemoryStore();
  /**
   * \brief Find which observations shall be moved from memory store 2 to error list
   */
  void checkSecondMemoryStore();
  /**
   * \return TRUE if given parameter has model values
   */
  bool paramHasModel(int);
  int paramIsCode(int); 
  /**
   * \return TRUE if given observation is missing
   */
  bool obsInMissList(mem);
  double calcdist(double, double, double, double);
  void swapRows(int, int, bool);
  void sortColumn(int, bool, bool);
  bool specialTimeFilter(int, miutil::miTime); 
  bool typeFilter(int, int, int, miutil::miTime); 
private slots:
  void tableCellClicked(int, int, int, const QPoint&);
  void updateFaillist(int, int);
  //void updateKvBase(int, int);
  void updateKvBase(mem*);
  void showFail(int, int, int, const QPoint&);

  void showSameStation();
  void markModified( int row, int col );
  void clearOtherMods( int row, int col );
  void setupMissingList( int row, int col );
  //  void setupWeather( int row, int col );
  //  void showWeather( ErrorList* );

  /**
   * \brief Identifies station obstime at row, col, and emits the 
   *        stationSelected signal.
   */
  void signalStationSelected( int row );
private:
  //typedef std::set<int> ModList;
  typedef std::set<ErrorListFirstCol *> ModList;
  typedef ModList::const_iterator CIModList;
  ModList modifiedRows;
  QMap<int, float> lowMap;
  QMap<int, float> highMap;

  const struct mem *getMem( int row ) const;
  kvalobs::kvData getKvData( const struct mem &m ) const;
  ExtendedFunctionalityHandler *efh;
  //  OkTableItem checkItem( int, int) const;
public:
  kvalobs::kvData getKvData( int row ) const;
  kvalobs::kvData getKvData( ) const { return getKvData( currentRow() ); }
  

public slots:
  void saveChanges();
  void printErrorList();
};

#endif
