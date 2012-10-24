/* -*- c++ -*-
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

#include "KvalobsData.h"
#include "KvalobsDataModel.h"

#include <kvalobs/kvData.h>
#include <kvalobs/kvObsPgm.h>

#include <QtCore/QString>
#include <QtGui/QValidator>
#include <Qt3Support/q3table.h>

#include <vector>

class ErrorListFirstCol;
class HqcMainWindow;
class miMessage;
class QMouseEvent;
class QPainter;
class QWidget;

namespace FailInfo {
class FailDialog;
}

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
	    const timeutil::ptime&,
	    const timeutil::ptime&,
	    QWidget*,
	    int,
	    int*,
	    std::vector<model::KvalobsData>&,
	    std::vector<modDatl>&,
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
    std::string controlinfo;
    std::string useinfo;
    std::string cfailed;
    int flg;
    int sen;
    int lev;
    QString flTyp;
    int parNo;
    QString parName;
    int stnr;
    QString name;
    timeutil::ptime obstime;
    timeutil::ptime tbtime;
    int typeId;
  };

  /*!
   * \brief
   */
  struct missObs {
    timeutil::ptime oTime;
    int time;
    int parno;
    int statno;
    int missNo;
  };

  /*!
   * \brief
   */
  std::vector<missObs> mList;

  /*!
   * \brief
   */
  std::vector<mem> missList;

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
  //  void stationSelected( int station, const timeutil::ptime & obstime );
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
  std::vector<mem> memStore1;
  /*!
   * \brief Temporary store for observations with these flags:
   * fr=4, fr=5, fs=2, fnum=4, fnum=5
   */
  std::vector<mem> memStore2;
  /*!
   * \brief
   */
  //  vector<mem> memStore3;

  std::vector<int> cP;

private:
  HqcMainWindow * mainWindow;
//  /**
//   * \brief Indexes of elements wchich are not transferred to the error list
//   */
//  std::vector<int> noError;
  /**
   * \brief Indexes of elements wchich are transferred to the error list
   */
  std::vector<int> error;
  /**
   * \brief Decide if an observation is going to the error list or not
   * \return The largest flag value from the automatic control, negative
   *         if no HQC control is indicated
   */
  int errorFilter(int, std::string, std::string, QString&);
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
   * \brief Find FF in memory store 2
   */
  double FF();

  double calcdist(double, double, double, double);
  void swapRows(int, int, bool);
  void sortColumn(int, bool, bool);
  /**
   * \brief Checks if given parameter can be stored at given time.
   */
  bool specialTimeFilter(int, const timeutil::ptime&);
  bool typeFilter(int, int, int, const timeutil::ptime&);
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
  //  OkTableItem checkItem( int, int) const;
};

#endif
