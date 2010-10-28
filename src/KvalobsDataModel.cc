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
#include <kvalobs/flag/kvControlInfo.h>
#include <QDebug>


namespace
{
  // Only for internal use
  struct InvalidIndex : public std::exception {};
}

namespace model
{
  const int KvalobsDataModel::COLUMNS_PER_PARAMETER = int(ColumnType_SENTRY);

  KvalobsDataModel::KvalobsDataModel(
      const std::vector<int> & parameters,
      QMap<int,QString> & paramIdToParamName,
      KvalobsDataListPtr datalist,
      const std::vector<modDatl> & modeldatalist,
      QObject * parent) :
    QAbstractTableModel(parent),
    kvalobsData_(datalist),
    modeldatalist_(modeldatalist)
  {
    for ( std::vector<int>::const_iterator param = parameters.begin(); param != parameters.end(); ++ param ) {
        QString paramName = "unknown";
        QMap<int,QString>::const_iterator find = paramIdToParamName.find(* param);
        if ( find != paramIdToParamName.end() )
          paramName = * find;
        parametersToShow_.push_back(Parameter(* param, paramName));
    }
    qDebug() << "Statistics\n"
        "--------\n"
        "Number of parameters: " << parametersToShow_.size();
    for ( std::vector<Parameter>::const_iterator it = parametersToShow_.begin(); it != parametersToShow_.end(); ++ it )
      qDebug() << it->paramid << ":\t" << qPrintable(it->parameterName);
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
    return kvalobsData_->size();
  }

  int KvalobsDataModel::columnCount(const QModelIndex & parent) const {
    return parametersToShow_.size() * COLUMNS_PER_PARAMETER;
  }

  QVariant KvalobsDataModel::data(const QModelIndex & index, int role) const {

    if ( not index.isValid() or index.row() >= kvalobsData_->size() )
      return QVariant();

    switch ( role ) {
    case Qt::DisplayRole:
        return displayRoleData(index);
    }
    return QVariant();
  }

  QVariant KvalobsDataModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if ( Qt::Horizontal == orientation) {
      if ( Qt::DisplayRole == role ) {
          const Parameter & parameter = getParameter_(section);
          switch (getColumnType_(section))
          {
          case Original:
            return QString("%1\norig").arg(parameter.parameterName);
          case Flag:
            return QString("%1\nflag").arg(parameter.parameterName);
          case Corrected:
            return QString("%1\ncorr").arg(parameter.parameterName);
          case Model:
            return QString("%1\nmodel").arg(parameter.parameterName);
          default:
            return QVariant();
          }
      }
    }
    else {
        Q_ASSERT(Qt::Vertical == orientation);
        if ( Qt::DisplayRole == role ) {
            const KvalobsData & d = kvalobsData_->at(section);
            // TODO: improve formatting
            QString display = "%1 - %2\t%3";
            const std::string date = d.otime().isoTime();
            return display.arg(QString::number(d.stnr()), d.name(), date.c_str());
        }
        else return QVariant();
    }
    return QVariant();
  }

  Qt::ItemFlags KvalobsDataModel::flags(const QModelIndex & index) const
  {
    Qt::ItemFlags ret = QAbstractTableModel::flags(index);
    if ( getColumnType_(index) == Corrected )
      ret |= Qt::ItemIsEditable;
    return ret;
  }

  bool KvalobsDataModel::setData(const QModelIndex & index, const QVariant & value, int role)
  {
    if ( not index.isValid() or index.row() >= kvalobsData_->size() )
      return false;

    if ( Qt::EditRole == role ) {
      if ( getColumnType_(index) == Corrected ) {
          const Parameter & p = getParameter_(index);

          bool ok;
          double val = value.toDouble(& ok);
          if ( not ok )
            return false;

          KvalobsData & d = kvalobsData_->at(index.row());
          if ( d.controlinfo(p.paramid).flag(kvQCFlagTypes::f_fmis) == 3 ) {
              kvalobs::kvControlInfo ci = d.controlinfo(p.paramid);
              ci.set(kvQCFlagTypes::f_fmis, 1);
              d.set_controlinfo(p.paramid, ci);
          }

          qDebug() << "Station " << d.stnr() << " (" << d.name() << "): Changed parameter " << qPrintable(p.parameterName) << " from " << d.corr(p.paramid) << " to " << val;

          d.set_corr(p.paramid, val);

          emit dataChanged(index, index);
          try {
            const kvalobs::kvData & d = getKvData_(index);
            emit dataModification(d);
          }
          catch ( InvalidIndex & ) {}
          return true;
      }
    }
    return false;
  }


  QVariant KvalobsDataModel::displayRoleData(const QModelIndex & index) const
  {
    const Parameter & p = getParameter_(index);

    const KvalobsData & d = kvalobsData_->at(index.row());
    const kvalobs::kvControlInfo & controlinfo = d.controlinfo(p.paramid);

    const int fmis = controlinfo.flag(kvQCFlagTypes::f_fmis);

    ColumnType columnType = getColumnType_(index);
    switch ( columnType )
    {
    case Original:
      {
        if ( fmis == 1 or fmis == 3 )
          return QVariant();
      return d.orig(p.paramid);
      }
    case Flag:
      {
        if ( fmis == 3 )
          return QVariant();
        return d.flag(p.paramid);
      }
    case Corrected:
      {
        if ( fmis == 2 or fmis == 3 )
          return QVariant();
      return d.corr(p.paramid);
      }
    case Model:
      {
        for ( std::vector<modDatl>::const_iterator it = modeldatalist_.begin(); it != modeldatalist_.end(); ++ it )
          if ( it->stnr == d.stnr() and it->otime == d.otime() ) {
            double ret = it->orig[p.paramid];
            if ( ret != -32767 )
              return ret;
            else
              return QVariant();
        }
        return QVariant();
      }
    default:
      return QVariant();
    }
  }

  const KvalobsDataModel::Parameter & KvalobsDataModel::getParameter_(const QModelIndex &index) const
  {
    return getParameter_(index.column());
  }

  const KvalobsDataModel::Parameter & KvalobsDataModel::getParameter_(int column) const
  {
    return parametersToShow_.at(column / COLUMNS_PER_PARAMETER);
  }

  KvalobsDataModel::ColumnType KvalobsDataModel::getColumnType_(const QModelIndex & index) const
  {
    return getColumnType_(index.column());
  }

  KvalobsDataModel::ColumnType KvalobsDataModel::getColumnType_(int column) const
  {
    switch ( column % COLUMNS_PER_PARAMETER ) {
    case 0:
      return Original;
    case 1:
      return Flag;
    case 2:
      return Corrected;
    case 3:
      return Model;
    }
    Q_ASSERT(false);
  }

  kvalobs::kvData KvalobsDataModel::getKvData_(const QModelIndex & index) const
  {
    if ( not index.isValid() or index.row() >= kvalobsData_->size() )
      throw InvalidIndex();

    const Parameter & parameter = getParameter_(index);

    const KvalobsData & d = kvalobsData_->at(index.row());
    return d.getKvData(parameter.paramid);
  }

}
