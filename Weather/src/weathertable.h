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
#ifndef __Weather__WeatherTable_h__
#define __Weather__WeatherTable_h__

#include "timeobs.h"
//#include "weatherdialog.h"
#include "dataconsistencyverifier.h"
#include "fdchecktableitem.h"
#include "tnchecktableitem.h"
#include "weathertableitem.h"
#include "flagitem.h"
#include "selfexplainable.h"
#include <decodeutility/DataReinserter.h>
#include <qtooltip.h>
//#include <qtable.h>
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

namespace Weather
{
  const int params[] = { 211,214,216,213,215,262,178,61,81,86,83,15,14,55,108,
			 109,110,112,18,7,273,41,31,32,33,42,43,34,36,38,40,
			 23,24,22,403,404,131,134,151,154,250,221,9,12};
  const int NP = 44;
  const int NL = 49;

  const QString horizonHeaders[] =
    {
      "TA", "TAN_12", "C", "TAX_12","C", "TAN", "TAX",
      "UU", "PR", "DD","FF", "FX", "FG","NN", "NH", "HL",
      "RR_6", "C", "RR_12","C", "RR_24", "C", "SA", "SD",
      "EM", "VV", "WW", "V1", "V2", "V3", "W1", "W2",
      "V4", "V5", "V6", "V7", "CL", "CM", "CH", "MDIR",
      "MSPE", "HW", "HWA", "PW", "PWA", "TW", "TG", "IR", "ITR"
    };
  /**
   * \brief Columns with data
   */
  const int datCol[] = {0,1,3,5,6,7,8,9,10,11,12,13,14,15,16,18,
			20,22,23,24,25,26,27,28,29,30,31,32,33,34,
			35,36,37,38,39,40,41,42,43,44,45,46,47,48};

  /**
   * \brief Columns with checkboxes
   */
  const int cbCol[]  = {2,4,17,19,21};
  /**
   * \brief Columns with possible distributed values
   */
  const int dbCol[]  = {1,2,14,15,16};

  /**
   * \brief Number of decimals in respective column
   */
  const int d1Par[]  = {1,1,1,1,1,0,1,0,1,1,1,0,0,0,1,1,1,1,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  class WeatherTableToolTip;


  class WeatherTable 
    : public QTable
  {
    friend class WeatherDialog;
    Q_OBJECT;

    //    void setup();

  public:
    /**
     * \throws std::runtime_error if error happens when contacting kvalobs.
     */
    //    WeatherTable(QToolTipGroup* ttGroup = 0, QWidget* parent = 0, int type = 0 );
    WeatherTable( QWidget* parent = 0, int type = 0 );
    virtual ~WeatherTable();
    void getModifiedData( DataConsistencyVerifier::DataSet & );    
    /**
     * \brief [Start, stop) dates for which to fetch data.
     */
    //    typedef std::pair<miutil::miTime, miutil::miTime> DateRange;

    /**
     * \brief Date range for editor
     */
   
    //    const DateRange & getDateRange() const { return dateRange; }
    //    int getStation() const { return station; }
    /*
    const miutil::miDate &getRefDate() const { return refDate; }
    int getType() const { return type; }
    int getSensor() const { return sensor; }
    int getLevel() const { return level; }
    
    void getModifiedData( DataConsistencyVerifier::DataSet & mod );

    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    */
  public slots:
    /**
     * \brief Save modified data
     */
    /*
    virtual bool saveData( const kvalobs::DataReinserter<kvservice::KvApp> *ri );
    */
    //    virtual void polish();
    
    /*
    virtual void headerMove( int section, int fromIndex, int toIndex );
    */
  protected:

    //    virtual void activateNextCell();

    /**
     * \brief Get start and stop dates for which to fetch data.
     */
    //    static WeatherTable::DateRange calculateDateRange( const miutil::miDate & refDate,
    //						  int daysToDisplay = 15 );

  //    virtual void displayData();

    /**
     * \brief Calculate the text for a single element of the vertical header.
     */
  //    virtual QString verticalHeaderText( const miutil::miDate &date ) const;

    //    TimeObsListPtr observations;

    /**
     * \brief Column number for sections
     */
  //    std::vector<int> toCol;

    /**
     * \brief Section number for columns
     */
  //    std::vector<int> toSec;
  
  private:
    std::vector<kvalobs::kvData> kvDatList;
    std::vector<kvalobs::kvData> kvCorrList;
    QMap<int, int> columnIndex;
    QMap<int, float> lowMap;
    QMap<int, float> highMap;
    typedef QPair<float,float> oldNewPair;
    std::vector<oldNewPair> oldNew;
    typedef QPair<int, int> rowColPair;
    std::vector<rowColPair> rowCol;
    QMap<int,QString> parm;
    int currRow;
    struct synDat {
      double sdat[NP];
      int styp[NP];
    };

    struct synFlg {
      QString sflg[NP];
    };

    struct Corrig {
      miutil::miTime oTime;
      QString parName;
      float oldVal;
      float newVal;
    };
    synDat sd;
    Corrig corr;
    int station;
    const miutil::miDate refDate;
    std::vector<miutil::miTime> timeList;
    //    DateRange dateRange;
    QString flagText(const std::string&);
    void displayHorizontalHeader();
    void displayVerticalHeader(std::vector<miutil::miTime>&);
    void displayData(QString, std::vector<synDat>&, std::vector<synFlg>&);
    void displayFlags(std::vector<synFlg>&);
    void readLimits();
    kvalobs::kvData getKvData(int, int);
    WeatherTableToolTip *toolTip;
    /*
    int type;
    int sensor;
    int level;

    void setupTable();
    void setColumnOrder();
    const std::vector<QString> & getHeaderOrder() const;
    
    //static const int daysToDisplay = 14;
    */
  protected slots:
    /**
     * \brief Update status bar with info from cell.
     */
    void restoreOld();
    void showCurrentPage( );
    
    virtual void updateStatusbar( int row, int col );
    void markModified( int, int );    
  public:
    //    WeatherTableToolTip *toolTip;
    //    QToolTipGroup *ttGroup;

  };
  
}

#endif // __Weather__WeatherTable_h__
