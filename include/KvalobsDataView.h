/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2010 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 email: kvalobs-dev@met.no

 This file is part of KVALOBS

 KVALOBS is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 KVALOBS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with KVALOBS; if not, write to the Free Software Foundation Inc.,
 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KVALOBSDATAVIEW_H_
#define KVALOBSDATAVIEW_H_

#include "timeutil.hh"

#include <QtGui/QTableView>

#include <set>

namespace model
{
  class KvalobsDataModel;

  class KvalobsDataView : public QTableView
  {
    Q_OBJECT
  public:
    template<class Iterator>
    KvalobsDataView(Iterator modelValuesStart, Iterator modelValuesStop, QWidget * parent = 0) :
      QTableView(parent), modelParameters_(modelValuesStart, modelValuesStop)
    {
      setup_();
    }

    virtual ~KvalobsDataView();

  public slots:
    void toggleShowFlags(bool show);
    void toggleShowOriginal(bool show);
    void toggleShowModelData(bool show);

    void selectStation(const QString & station);
    void selectStation(int stationid, const timeutil::ptime& obstime);
    void selectTime(const timeutil::ptime& obstime);

  signals:
    void stationSelected(int stationid, const timeutil::ptime& obstime);
    void timeSelected(const timeutil::ptime& obstime);
    void parameterSelected(const QString & parameterName);

  protected slots:
    virtual void currentChanged(const QModelIndex & current, const QModelIndex & previous);

  private:
    const KvalobsDataModel * getModel_() const;
    void setup_();

    const std::set<int> modelParameters_;
  };

}

#endif /* KVALOBSDATAVIEW_H_ */
