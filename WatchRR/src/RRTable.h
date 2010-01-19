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
#ifndef __WatchRR__RRTable_h__
#define __WatchRR__RRTable_h__

#include "DayObs.h"
#include "dataconsistencyverifier.h"
#include "RRTableItem.h"
#include <decodeutility/DataReinserter.h>
#include <qtooltip.h>
#include <q3table.h>
#include <puTools/miDate>
#include <utility>
#include <vector>
#include <set>

namespace kvalobs
{
  class kvStation;
}
namespace kvservice
{
  class KvApp;
}

namespace WatchRR
{
  class RRTableToolTip;


  class RRTable 
    : public Q3Table
  {
    Q_OBJECT;

    void setup();

  public:
    /**
     * \throws std::runtime_error if error happens when contacting kvalobs.
     */
    RRTable( int station, const miutil::miDate &date, 
	     int type, int sensor, int level,
	     QWidget *parent = 0, const char *name = 0 );

    RRTable( DayObsListPtr dayobs,
	     QWidget *parent = 0, const char *name = 0 );

    virtual ~RRTable();

    /**
     * \brief [Start, stop) dates for which to fetch data.
     */
    typedef std::pair<miutil::miDate, miutil::miDate> DateRange;

    /**
     * \brief Date range for editor
     */
    const DateRange & getDateRange() const { return dateRange; }

    int getStation() const { return station; }
    const miutil::miDate &getRefDate() const { return refDate; }
    int getType() const { return type; }
    int getSensor() const { return sensor; }
    int getLevel() const { return level; }
    
    void getModifiedData( DataConsistencyVerifier::DataSet & mod );

    virtual QSize sizeHint() const;
    //    virtual QSizePolicy sizePolicy() const;

  public slots:
    /**
     * \brief Save modified data
     */
    virtual bool saveData( const kvalobs::DataReinserter<kvservice::KvApp> *ri );
    
    //    virtual void ensurePolished();

  protected slots:
    /**
     * \brief Update status bar with info from cell.
     */
    virtual void updateStatusbar( int row, int col );

    virtual void headerMove( int section, int fromIndex, int toIndex );

  protected:

    virtual void activateNextCell();

    /**
     * \brief Get start and stop dates for which to fetch data.
     */
    static RRTable::DateRange calculateDateRange( const miutil::miDate & refDate,
						  int daysToDisplay = 15 );

    virtual void displayData();

    /**
     * \brief Calculate the text for a single element of the vertical header.
     */
    virtual QString verticalHeaderText( const miutil::miDate &date ) const;

    DayObsListPtr observations;

    /**
     * \brief Column number for sections
     */
    std::vector<int> toCol;

    /**
     * \brief Section number for columns
     */
    std::vector<int> toSec;

  private:
    int station;
    const miutil::miDate refDate;
    DateRange dateRange;
    int type;
    int sensor;
    int level;

    void setupTable();
    void setColumnOrder();
    const std::vector<QString> & getHeaderOrder() const;
    
    //static const int daysToDisplay = 14;

  public:
    RRTableToolTip *toolTip;
    //    QToolTipGroup *ttGroup;

  };
  
}

#endif // __WatchRR__RRTable_h__
