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

#include <QWidget>

#include <set>

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <q3table.h>
#include <qstring.h>
#include <qpainter.h>
#include <puTools/miTime.h>
#include <puTools/miString.h>
#include <qUtilities/miMessage.h>
#include <math.h>
#include "hqcdefs.h"
#include <kvalobs/kvData.h>
#include <qvalidator.h>

#include <FailDialog.h>

class ErrorListFirstCol;

using namespace std;
using namespace kvalobs;

class QMouseEvent;
//class QEvent;
class HqcMainWindow;
class ExtendedFunctionalityHandler;

const float rejectedValue_ = -32767.0;
const float discardedValue_ = -32766.0;
const int npnc = 106;
const int npcc = 83;
//const int npcc = 27;
//const int npcc = 32;
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
/*
const QString controlNoControl[] = {"QC1-2-96:1","QC1-2-97:1","QC1-2-100:1",
				    "QC1-2-101:1","QC1-2-105:1","QC1-2-123A:1",
				    "QC1-2-142:1","QC1-2-143:1","QC1-2-144:1",
				    "QC1-2-145:1","QC1-2-146:1","QC1-2-147:1",
				    "QC1-2-148:1","QC1-2-149:1","QC1-2-150:1",
				    "QC1-2-151:1","QC1-2-152:1","QC1-2-153:1",
				    "QC1-2-154:1","QC1-2-155:1","QC1-2-156:1",
				    "QC1-2-158:1","QC1-2-159:1","QC1-2-160:1",
				    "QC1-2-161:1","QC1-2-162:1","QC1-2-21:1"};
				    //				    "QC1-4-81:1","QC1-4-109:1","QC1-4-178:1",
				    //				    "QC1-4-211:1","QC1-4-262:1"};
				    */
/*
const QString controlNoControl[] = {"QC1-2-100:1","QC1-2-123A:1","QC1-2-123B:1","QC1-2-123C:1","QC1-2-124A:1",
				    "QC1-2-125A:1","QC1-2-126B:1","QC1-2-129A:1","QC1-2-129B:1","QC1-2-130A:1",
				    "QC1-2-131:1","QC1-2-132:1","QC1-2-133:1","QC1-2-134B:1","QC1-2-139A:1",
				    "QC1-2-139B:1","QC1-2-142:1","QC1-2-143:1","QC1-2-144:1","QC1-2-145:1",
				    "QC1-2-146A:1","QC1-2-146B:1","QC1-2-146C:1","QC1-2-146D:1","QC1-2-147_148A:1",
				    "QC1-2-147_148B:1","QC1-2-147_148C:1","QC1-2-147_148D:1","QC1-2-149A:1",
				    "QC1-2-149B:1","QC1-2-149C:1","QC1-2-149D:1","QC1-2-150A:1","QC1-2-150B:1",
				    "QC1-2-150C:1","QC1-2-150D:1","QC1-2-151A:1","QC1-2-151B:1","QC1-2-151C:1",
				    "QC1-2-151D:1","QC1-2-152:1","QC1-2-153:1","QC1-2-154:1","QC1-2-155:1",
				    "QC1-2-156A:1","QC1-2-156B:1","QC1-2-156C:1","QC1-2-156D:1","QC1-2-158A:1",
				    "QC1-2-158B:1","QC1-2-158C:1","QC1-2-158D:1","QC1-2-159A:1","QC1-2-159B:1",
				    "QC1-2-159C:1","QC1-2-159D:1","QC1-2-160A:1","QC1-2-160B:1", "QC1-2-160C:1",
				    "QC1-2-160D:1","QC1-2-161A:1","QC1-2-161B:1","QC1-2-161C:1","QC1-2-161D:1",
				    "QC1-2-162A:1","QC1-2-162B:1","QC1-2-162C:1","QC1-2-162D:1","QC1-2-163A:1",
				    "QC1-2-163B:1","QC1-2-163C:1","QC1-2-163D:1","QC1-2-164:1","QC1-2-165:1",
				    "QC1-2-166:1","QC1-2-167:1","QC1-2-168:1","QC1-2-169:1","QC1-2-170:1",
				    "QC1-2-171:1","QC1-2-172:1","QC1-2-173:1","QC1-2-175:1"};
*/
const QString controlNoControl[] = {"QC1-2-100","QC1-2-123A","QC1-2-123B","QC1-2-123C","QC1-2-124A",
				    "QC1-2-125A","QC1-2-126B","QC1-2-129A","QC1-2-129B","QC1-2-130A",
				    "QC1-2-131","QC1-2-132","QC1-2-133","QC1-2-134B","QC1-2-139A",
				    "QC1-2-139B","QC1-2-142","QC1-2-143","QC1-2-144","QC1-2-145",
				    "QC1-2-146A","QC1-2-146B","QC1-2-146C","QC1-2-146D","QC1-2-147_148A",
				    "QC1-2-147_148B","QC1-2-147_148C","QC1-2-147_148D","QC1-2-149A",
				    "QC1-2-149B","QC1-2-149C","QC1-2-149D","QC1-2-150A","QC1-2-150B",
				    "QC1-2-150C","QC1-2-150D","QC1-2-151A","QC1-2-151B","QC1-2-151C",
				    "QC1-2-151D","QC1-2-152","QC1-2-153","QC1-2-154","QC1-2-155",
				    "QC1-2-156A","QC1-2-156B","QC1-2-156C","QC1-2-156D","QC1-2-158A",
				    "QC1-2-158B","QC1-2-158C","QC1-2-158D","QC1-2-159A","QC1-2-159B",
				    "QC1-2-159C","QC1-2-159D","QC1-2-160A","QC1-2-160B", "QC1-2-160C",
				    "QC1-2-160D","QC1-2-161A","QC1-2-161B","QC1-2-161C","QC1-2-161D",
				    "QC1-2-162A","QC1-2-162B","QC1-2-162C","QC1-2-162D","QC1-2-163A",
				    "QC1-2-163B","QC1-2-163C","QC1-2-163D","QC1-2-164","QC1-2-165",
				    "QC1-2-166","QC1-2-167","QC1-2-168","QC1-2-169","QC1-2-170",
				    "QC1-2-171","QC1-2-172","QC1-2-173","QC1-2-175"};
/**
 * \brief Cells in the errorlist where the user can insert values.
 */

class CrTableItem : public Q3TableItem {
  bool numbers;
public:
  CrTableItem(Q3Table* t,
	      EditType et,
	      const QString &txt,
	      bool acceptNumbers)
    : Q3TableItem( t, et, txt )
    , numbers(acceptNumbers) {}
  virtual ~CrTableItem() { }
  void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );

  virtual QWidget *createEditor() const;
  static const QRegExpValidator validator;
  static const QRegExp re;
};

/**
 * \brief Checkable cells in the errorlist where the user can approve or reject an observation.
 */

class OkTableItem : public Q3CheckTableItem {
public:
  OkTableItem(Q3Table* t,
	      const QString &txt ) : Q3CheckTableItem( t, txt ) {}
  void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
};

/**
 * \brief Cells in the errorlist with the observation data.
 */

class DataCell : public Q3TableItem {
public:
  DataCell(Q3Table* t,
	      EditType et,
	      const QString &txt ) : Q3TableItem( t, et, txt ) {
  }
  QString key() const;
  void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
};

/**
 * \brief The error list. i.e. list of observations with error flags.
 *
 * \detailed The error list consists of two parts.  One part holds the data for the observations
 * which are flagged as erronous.  The other part has cells where the user can insert
 * new values or approve or reject existing values.
 */

class ErrorList : public Q3Table {
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
	    vector<model::KvalobsData>&,
	    vector<modDatl>&,
	    list<kvStation>&,
	    int,
	    int,
	    QString&);
  virtual ~ErrorList();

  /*!
   * \brief Reads the climatological limits from a file
   */
  void readLimits();

  /*!
   * \brief
   */
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

  /*!
   * \brief
   */
  vector<missObs> mList;

  /*!
   * \brief
   */
  vector<mem> missList;

  /**
   * \brief Prompt user to save data.
   * \returns true if user did not press cancel.
   */
  bool maybeSave();

public:
  /*!
   * \brief Constructs a kvData object
   * \return The kvData object corresponding to the given row in the error list
   */
  kvalobs::kvData getKvData( int row ) const;
  /*!
   * \brief Constructs a kvData object
   * \return The kvData object corresponding to the current row in the error list
   */
  kvalobs::kvData getKvData( ) const { return getKvData( currentRow() ); }


public slots:
  /*!
   * \brief Updates controlinfo and sends the changed data to the kvalobs database
   */
  void saveChanges();
  //  void printErrorList();


signals:

  /**
   * \brief Reports the selection of a new station and/or obstime in the
   *        errorlist.
   */
  //  void stationSelected( int station, const miutil::miTime & obstime );
  void statSel( miMessage& letter );

  /**
   * \brief Reports the closing of the
   *        errorlist.
   */
  void errorListClosed();

protected:
  /*!
   * \brief
   */
  virtual bool event( QEvent * e );
  void closeEvent ( QCloseEvent * event );


private:
  /*!
   * \brief
   */
  int stationidCol;
  /*!
   * \brief
   */
  int typeidCol;

  /*!
   * \brief
   */
  struct refs {
    int stnr;
    int rstnr;
    int parNo;
    double dist;
  };
  /*!
   * \brief
   */
  QString opName;
  FailInfo::FailDialog* fDlg;
  std::list<kvalobs::kvObsPgm> obsPgmList;
  std::list<long> statList;
  /*!
   * \brief Temporary store for observations with these flags:
   * fr=2, fr=3, fcc=2, fcp = 2, fnum=2, fnum=3
   */
  vector<mem> memStore1;
  /*!
   * \brief Temporary store for observations with these flags:
   * fr=4, fr=5, fs=2, fnum=4, fnum=5
   */
  vector<mem> memStore2;
  /*!
   * \brief
   */
  //  vector<mem> memStore3;

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
   * \brief Checks if a parameter has model values.
   *
   * \return TRUE if given parameter has model values
   */
  bool paramHasModel(int);
  /**
   * \brief Checks if a parameter is code.
   *
   * \return 0 if given parameter is code, 1 otherwise.
   */
  int paramIsCode(int);
  /**
   * \brief Checks if an observation is in the missing list
   *
   * \return TRUE if observation is missing.
   */
  bool obsInMissList(mem);
  /**
   * \brief Checks if a station is a coast/maritime or inland station
   *
   * \return TRUE if coast/maritime station
   */
  bool isCoastStation(int stnr);
  /**
   * \brief Find FF in memory store 2
   */
  double FF();

  double calcdist(double, double, double, double);
  void swapRows(int, int, bool);
  void sortColumn(int, bool, bool);
  /**
   * \brief Checks if given parameter can be stored at given time.
   */
  bool specialTimeFilter(int, miutil::miTime);
  bool typeFilter(int, int, int, miutil::miTime);
private slots:
  //  void tableCellClicked(int, int, int, const QPoint&, vector<model::KvalobsData>&);
  void tableCellClicked(int, int, int);
  void updateFaillist(int, int);
  //void updateKvBase(int, int);
  void updateKvBase(mem*);
  void showFail(int, int, int, const QPoint&);

  void showSameStation();
  void markModified( int row, int col );
  void clearOtherMods( int row, int col );
  //  void setupMissingList( int row, int col );
  void setupMissingList();
  void execMissingList();
  //  void setupWeather( int row, int col );
  //  void showWeather( ErrorList* );

  /**
   * \brief Identifies station obstime at row, col, and emits the
   *        stationSelected signal.
   */
  void signalStationSelected( int row );
private:
  int selectedRow;
  //typedef std::set<int> ModList;
  typedef std::set<ErrorListFirstCol *> ModList;
  typedef ModList::const_iterator CIModList;
  ModList modifiedRows;
  QMap<int, float> lowMap;
  QMap<int, float> highMap;

  /*!
   * \brief Constructs a memory store object
   * \return The memory store object corresponding to the given row in the error list
   */
  const struct mem *getMem( int row ) const;
  /*!
   * \brief Constructs a kvData object from a memory store object
   */
  kvalobs::kvData getKvData( const struct mem &m ) const;
  ExtendedFunctionalityHandler *efh;
  //  OkTableItem checkItem( int, int) const;
};

#endif
