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

#include <KvalobsDataModel.h>


namespace model
{
  const int KvalobsDataModel::COLUMNS_PER_PARAMETER = 3;


  KvalobsDataModel::KvalobsDataModel(KvalobsDataList & kvalobsData, QObject * parent) :
    QAbstractTableModel(parent),
    kvalobsData_(kvalobsData)
  {
  }

  KvalobsDataModel::~KvalobsDataModel()
  {
  }

//  QModelIndex KvalobsDataModel::index(int row, int column, const QModelIndex & parent) const {
//    return QModelIndex();
//  }
//
//  QModelIndex KvalobsDataModel::parent(const QModelIndex & index) const {
//    return QModelIndex();
//  }

  int KvalobsDataModel::rowCount(const QModelIndex & parent) const {
    return kvalobsData_.size();
  }

  int KvalobsDataModel::columnCount(const QModelIndex & parent) const {
    return 1043 * COLUMNS_PER_PARAMETER;
  }

  QVariant KvalobsDataModel::data(const QModelIndex & index, int role) const {

    if ( not index.isValid() or index.row() >= kvalobsData_.size() )
      return QVariant();

    if ( Qt::DisplayRole == role ) {
        int parameterIdx = index.column() / COLUMNS_PER_PARAMETER;

        const KvalobsData & d = kvalobsData_[index.row()];
        std::string controlinfo = d.controlinfo(parameterIdx);
        switch (index.column() % COLUMNS_PER_PARAMETER)
        {
        case 0:
          {
            if ( controlinfo.substr(6,1) == "1" or controlinfo.substr(6,1) == "3" )
              return QVariant();
          return d.orig(parameterIdx);
          }
        case 1:
          {
            if ( controlinfo.substr(6,1) == "3" )
              return QVariant();
            return d.flag(parameterIdx);
          }
        case 2:
          {
            if ( controlinfo.substr(6,1) == "2" or controlinfo.substr(6,1) == "3" )
              return QVariant();
          return d.corr(parameterIdx);
          }
        default:
          return QVariant();
        }

    }
    return QVariant();
  }

  QVariant KvalobsDataModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if ( Qt::Horizontal == orientation) {
      if ( Qt::DisplayRole == role ) {
          QString parameterName = "RR_24";
          switch (section % COLUMNS_PER_PARAMETER)
          {
          case 0:
            return QString("%1 orig").arg(parameterName);
          case 1:
            return QString("%1 flag").arg(parameterName);
          case 2:
            return QString("%1 corr").arg(parameterName);
          default:
            return QVariant();
          }
      }
    }
    else {
        Q_ASSERT(Qt::Vertical == orientation);
        if ( Qt::DisplayRole == role ) {
            const KvalobsData & d = kvalobsData_[section];
            // TODO: improve formatting
            QString display = "%1 - %2\t%3";
            const std::string date = d.otime().isoTime();
            return display.arg(QString::number(d.stnr()), d.name(), date.c_str());
        }
        else return QVariant();
    }
    return QVariant();
  }

}
