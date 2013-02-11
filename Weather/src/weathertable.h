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
#ifndef __Weather__WeatherTable_h__
#define __Weather__WeatherTable_h__

#include "dataconsistencyverifier.h"
#include "timeutil.hh"

#include <QtCore/QMap>
#include <Qt3Support/Q3Table>

#include <vector>
#include <list>

namespace kvalobs
{
  class kvStation;
  class kvObsPgm;
}
namespace kvservice
{
  class KvApp;
}

namespace Weather
{

const int NP = 49;
extern const int params[NP];

  class WeatherTableToolTip;


  class WeatherTable
    : public Q3Table
  {
    friend class WeatherDialog;
    Q_OBJECT;

  public:
    /**
     * \throws std::runtime_error if error happens when contacting kvalobs.
     */
    WeatherTable( QWidget* parent = 0, QString name = "", int type = 0 );
    virtual ~WeatherTable();
    void getModifiedData( DataConsistencyVerifier::DataSet & );

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
      int ssen[NP];
      std::string ctrlinfo[NP];
    };

    struct synFlg {
      QString sflg[NP];
    };

    struct Corrig {
      timeutil::ptime oTime;
      QString parName;
      float oldVal;
      float newVal;
    };
    synDat sd;
    Corrig corr;
    int station;
    const timeutil::pdate refDate;
    std::vector<timeutil::ptime> timeList;
    QString flagText(const std::string&);
    void displayHorizontalHeader();
    void displayVerticalHeader(std::vector<timeutil::ptime>&);
    void displayData(QString, std::vector<synDat>&, std::vector<synFlg>&);
    void displayFlags(std::vector<synFlg>&);
    void readLimits();
    kvalobs::kvData getKvData(int, int);
    WeatherTableToolTip *toolTip;
    int findTypeId(int typ, int pos, int par, const timeutil::ptime& oTime);
  protected slots:
    /**
     * \brief Update status bar with info from cell.
     */
    void restoreOld();
    void showCurrentPage( );

    virtual void updateStatusbar( int row, int col );
    void markModified( int, int );
  };

}

#endif // __Weather__WeatherTable_h__
