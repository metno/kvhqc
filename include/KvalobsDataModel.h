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

#ifndef KVALOBSDATAMODEL_H_
#define KVALOBSDATAMODEL_H_


#include <QAbstractTableModel>
#include <vector>
#include "KvalobsData.h"

namespace model
{
  class KvalobsDataModel : public QAbstractTableModel
  {
    Q_OBJECT
  public:
    KvalobsDataModel(KvalobsDataList & kvalobsData, QObject * parent = 0);
    virtual ~KvalobsDataModel();

//    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;

//    virtual QModelIndex parent(const QModelIndex & index) const;

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  private:
    KvalobsDataList & kvalobsData_;

    static const int COLUMNS_PER_PARAMETER;
  };

}

#endif /* KVALOBSDATAMODEL_H_ */
