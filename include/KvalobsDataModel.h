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


#include "KvalobsData.h"
#include "hqcdefs.h"
#include <QAbstractTableModel>
#include <QString>
#include <vector>

namespace model
{
  class KvalobsDataModel : public QAbstractTableModel
  {
    Q_OBJECT
  public:
    KvalobsDataModel(
        const std::vector<int> & parameters,
        QMap<int,QString> & paramIdToParamName,
        KvalobsDataListPtr datalist,
        const std::vector<modDatl> & modeldatalist,
        bool showStationNameInHeader,
        bool showPositionInHeader,
        QObject * parent = 0);

    virtual ~KvalobsDataModel();

    KvalobsDataListPtr & kvalobsData() { return kvalobsData_; }
    const KvalobsDataListPtr & kvalobsData() const { return kvalobsData_; }

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex & index) const;

    bool setData(const QModelIndex &index, const QVariant & value, int role = Qt::EditRole);

    enum ColumnType
    {
      Original, Flag, Corrected, Model,
      ColumnType_SENTRY
    };
    ColumnType getColumnType(const QModelIndex & index) const;
    ColumnType getColumnType(int column) const;

    struct Parameter
    {
      Parameter(int paramid, const QString & parameterName) :
        paramid(paramid), parameterName(parameterName) {}

      int paramid;
      QString parameterName;
    };
    const Parameter & getParameter(const QModelIndex & index) const;
    const Parameter & getParameter(int column) const;


  public slots:
    void setShowStationName(bool show);
    void setShowPosition(bool show);

  signals:
    void dataModification(const kvalobs::kvData & modifiedData);

  private:
    QVariant displayRoleData(const QModelIndex & index) const;
    QVariant textColorRoleData(const QModelIndex & index) const;

    kvalobs::kvData getKvData_(const QModelIndex & index) const;

    KvalobsDataListPtr kvalobsData_;
    const std::vector<modDatl> & modeldatalist_;

    std::vector<Parameter> parametersToShow_;

    bool showStationNameInHeader_;
    bool showPositionInHeader_;

    static const int COLUMNS_PER_PARAMETER;
  };

}

#endif /* KVALOBSDATAMODEL_H_ */
