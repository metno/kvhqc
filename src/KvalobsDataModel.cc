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

#include "KvalobsDataModel.h"
#include "hqcmain.h"
#include "FunctionLogger.hh"
#include "mi_foreach.hh"
#include "timeutil.hh"

#include <kvalobs/flag/kvControlInfo.h>
#include <kvalobs/kvDataOperations.h>
#include <QtCore/QDebug>
#include <cmath>


namespace
{
  // Only for internal use
  struct InvalidIndex : public std::exception {};
}

namespace model
{
  const int KvalobsDataModel::COLUMNS_PER_PARAMETER = int(ColumnType_SENTRY);

  const int codeParam[] = {   1,  2,  3,  4,  6,  7,  9, 10, 11, 12,
			     13, 14, 15, 17, 18, 19, 20, 21, 22, 23,
			     24, 25, 26, 27, 27, 28, 31, 32, 33, 34,
			     35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
			     45, 46, 47, 48, 49, 54, 55, 56, 57,123,
			     124,128151,273,301,302,303,304,305,306,
			     307,308,1021,1022,1025,1026};

  KvalobsDataModel::KvalobsDataModel(
      const std::vector<int> & parameters,
      QMap<int,QString> & paramIdToParamName,
      KvalobsDataListPtr datalist,
      const std::vector<modDatl> & modeldatalist,
      bool showStationNameInHeader,
      bool showPositionInHeader,
      bool showHeightInHeader,
      bool editable,
      QObject * parent) :
    QAbstractTableModel(parent),
    kvalobsData_(datalist),
    modeldatalist_(modeldatalist),
    showStationNameInHeader_(showStationNameInHeader),
    showPositionInHeader_(showPositionInHeader),
    showHeightInHeader_(showHeightInHeader),
    correctedValuesAreEditable_(editable)
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

  const KvalobsData * KvalobsDataModel::kvalobsData(const QModelIndex & location) const
  {
    if ( not location.isValid() )
      return 0;
    int index = location.row();
    if ( index >= 0 and index < (int)kvalobsData_->size() )
      return & (*kvalobsData_)[index];
    return 0;
  }


//  QModelIndex KvalobsDataModel::index(int row, int column, const QModelIndex & parent) const {
//    return QModelIndex();
//  }
//
//  QModelIndex KvalobsDataModel::parent(const QModelIndex & index) const {
//    return QModelIndex();
//  }

  int KvalobsDataModel::rowCount(const QModelIndex & /*parent*/) const {
    return kvalobsData_->size();
  }

  int KvalobsDataModel::columnCount(const QModelIndex & /*parent*/) const {
    return parametersToShow_.size() * COLUMNS_PER_PARAMETER;
  }

  QVariant KvalobsDataModel::data(const QModelIndex & index, int role) const {

      if ( not index.isValid() or index.row() >= (int)kvalobsData_->size() )
      return QVariant();

    switch ( role ) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return displayRoleData(index);
    case Qt::TextColorRole:
      return textColorRoleData(index);
    case Qt::TextAlignmentRole:
      return textAlignmentRoleData(index);

    }
    return QVariant();
  }

  QVariant KvalobsDataModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if ( Qt::Horizontal == orientation) {
      if ( Qt::DisplayRole == role ) {
          const Parameter & parameter = getParameter(section);
          switch (getColumnType(section))
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

            QString header = QString::number(d.stnr());
            header += " - ";
            if ( showStationNameInHeader_ )
              header += d.name() + "\t";
            if ( showPositionInHeader_ ) {
                header += ' ' + QString::number(d.latitude(), 'f', 2) + ' ';
                header += QString::number(d.longitude(), 'f', 2) + ' ';
            }
            if ( showHeightInHeader_ ) {
	       header += ' ' + QString::number(d.altitude(), 'f', 0) + "m ";
	    }
            const std::string date = timeutil::to_iso_extended_string(d.otime());
            header += date.c_str();
	    const QString synopNumber = QString::number(d.snr());
	    header += ' ' + synopNumber;

	    HqcMainWindow * hqcm = getHqcMainWindow(this);
	    if ( hqcm->isShTy ) {
	      const QString typeId = QString::number(d.showTypeId());
	      header += ' ' + typeId;
	    }
            return header;
        }
        else return QVariant();
    }
    return QVariant();
  }

  Qt::ItemFlags KvalobsDataModel::flags(const QModelIndex & index) const
  {
    Qt::ItemFlags ret = QAbstractTableModel::flags(index);
    if ( correctedValuesAreEditable_ and getColumnType(index) == Corrected )
      ret |= Qt::ItemIsEditable;
    return ret;
  }


  bool KvalobsDataModel::setData(const QModelIndex & index, const QVariant & value, int role)
  {
      if ( not index.isValid() or index.row() >= (int)kvalobsData_->size() )
      return false;

    if ( Qt::EditRole == role ) {
      if ( getColumnType(index) == Corrected ) {
	const Parameter & p = getParameter(index);

	// Invalid input (should never happen, except when user entered empty string)
	bool ok;
	double val = value.toDouble(& ok);
	if ( not ok ) {
	  if ( value.toString().isEmpty() )
	    val = -32766;
	  else
	    return false;
	}

	try {
	  KvalobsData & d = kvalobsData_->at(index.row());

	  // Insignificant change. Ignored.
	  double oldValue = d.corr(p.paramid);
	  if ( std::fabs(oldValue - val) < 0.005 )
	    return false;

	  double oldOrig = d.orig(p.paramid);
	  kvalobs::kvControlInfo ctr = d.controlinfo(p.paramid);
	  int typ = d.typeId(p.paramid);

	  kvalobs::kvData changeData = getKvData_(index);

	  //	  if ( val != -32766 )
	  //	    kvalobs::hqc::hqc_auto_correct(changeData, val);
	  //	  else
	  //	    kvalobs::hqc::hqc_reject(changeData);
	  if ( val != -32766 && fabs(oldOrig - val) > 0.0001 ) {
	    kvalobs::hqc::hqc_auto_correct(changeData, val);
	  }
	  else if ( val != -32766 ) {
	    ctr.set(kvalobs::flag::fmis,0);
	    changeData.controlinfo(ctr);
	    kvalobs::hqc::hqc_accept(changeData);
	  }
	  else
	    kvalobs::hqc::hqc_reject(changeData);
	  std::string cfailed = changeData.cfailed();
	  if ( not cfailed.empty() )
	    cfailed += ",";
	  cfailed += "hqc";
	  changeData.cfailed(cfailed);

	  // Update stored data
	  d.set_corr(p.paramid, val);
	  d.set_controlinfo(p.paramid, changeData.controlinfo());
	  d.set_useinfo(p.paramid, changeData.useinfo());
	  if (abs(typ) > 999) {
	    HqcMainWindow * hqcm = getHqcMainWindow(this);
	    typ = hqcm->findTypeId(typ, d.stnr(), p.paramid, d.otime());
	    changeData.typeID(typ);
	  }

	  QModelIndex flagIndex = createIndex(index.row(), index.column() -1, 0);
	  emit dataChanged(flagIndex, index);
	  emit dataModification(changeData);
	  qDebug() << "SetData Station " << d.stnr() << " (" << d.name() << "): Changed parameter "
		   << qPrintable(p.parameterName) << " from " << oldValue
		   << " to " << val << "," << d.typeId(p.paramid);
	  cerr << changeData.useinfo() << endl;
	  return true;
	}
	catch ( InvalidIndex & ) {
	  return false;
	}
      }
    }
    return false;
  }

  bool KvalobsDataModel::setAcceptedData(const QModelIndex & index, const QVariant & value, bool maybeQC2, int role)
  {
      if ( not index.isValid() or index.row() >= (int)kvalobsData_->size() )
      return false;

    if ( Qt::EditRole == role ) {
      if ( getColumnType(index) == Corrected ) {
          const Parameter & p = getParameter(index);

          // Invalid input (should never happen, except when user entered empty string)
          bool ok;
          double val = value.toDouble(& ok);
          if ( not ok ) {
              if ( value.toString().isEmpty() )
                val = -32766;
              else
                return false;
          }

          try {
            KvalobsData & d = kvalobsData_->at(index.row());
	    kvalobs::kvControlInfo ctr = d.controlinfo(p.paramid);

            // Insignificant change. Ignored.
            double oldValue = d.corr(p.paramid);
            if ( std::fabs(oldValue - val) < 0.005 or oldValue == -32767 )
              return false;

	    int typ = d.typeId(p.paramid);

	    if (abs(typ) > 999) {
	      HqcMainWindow * hqcm = getHqcMainWindow(this);
	      typ = hqcm->findTypeId(typ, d.stnr(), p.paramid, d.otime());
	      d.set_typeId(p.paramid, typ);
	    }
            kvalobs::kvData changeData = getKvData_(index);
	    ctr.set(kvalobs::flag::fmis,0);
	    if ( val == -32767 ) {
	      if ( maybeQC2 )
		ctr.set(kvalobs::flag::fhqc,4);
	      else
		ctr.set(kvalobs::flag::fhqc,3);
	      ctr.set(kvalobs::flag::fmis,3);
       	      changeData.controlinfo(ctr);
	    }
	    else {
	      changeData.controlinfo(ctr);
	      kvalobs::hqc::hqc_accept(changeData);
	    }
            std::string cfailed = changeData.cfailed();
            if ( not cfailed.empty() )
              cfailed += ",";
            cfailed += "hqc";
            changeData.cfailed(cfailed);
	    changeData.corrected(val);
            // Update stored data
            d.set_corr(p.paramid, val);
            d.set_controlinfo(p.paramid, changeData.controlinfo());
            d.set_useinfo(p.paramid, changeData.useinfo());

            QModelIndex flagIndex = createIndex(index.row(), index.column() -1, 0);
            emit dataChanged(flagIndex, index);
            emit dataModification(changeData);
            qDebug() << "SetAcceptedData Station " << d.stnr() << " (" << d.name() << "): Changed parameter "
		     << qPrintable(p.parameterName) << " from "
		     << oldValue << " to " << val;
	    cerr << changeData.useinfo() << endl;
            return true;
          }
          catch ( InvalidIndex & ) {
              return false;
          }
      }
    }
    return false;
  }


  bool KvalobsDataModel::setDiscardedData(const QModelIndex & index, const QVariant & value, int role)
  {
      if ( not index.isValid() or index.row() >= (int)kvalobsData_->size() )
      return false;

    if ( Qt::EditRole == role ) {
      if ( getColumnType(index) == Corrected ) {
          const Parameter & p = getParameter(index);

          // Invalid input (should never happen, except when user entered empty string)
          bool ok;
          double val = value.toDouble(& ok);
          if ( not ok ) {
              if ( value.toString().isEmpty() )
                val = -32766;
              else
                return false;
          }

          try {
            KvalobsData & d = kvalobsData_->at(index.row());

            // Insignificant change. Ignored.
            double oldValue = d.corr(p.paramid);
            if ( std::fabs(oldValue - val) < 0.005 or oldValue == -32767 )
              return false;

	    int typ = d.typeId(p.paramid);

	    if (abs(typ) > 999) {
	      HqcMainWindow * hqcm = getHqcMainWindow(this);
	      typ = hqcm->findTypeId(typ, d.stnr(), p.paramid, d.otime());
	      d.set_typeId(p.paramid, typ);
	    }
            kvalobs::kvData changeData = getKvData_(index);
            if ( val != -32766 )
              kvalobs::hqc::hqc_auto_correct(changeData, val);
            else
              kvalobs::hqc::hqc_reject(changeData);
            std::string cfailed = changeData.cfailed();
            if ( not cfailed.empty() )
              cfailed += ",";
            cfailed += "hqc";
            changeData.cfailed(cfailed);

            // Update stored data
            d.set_corr(p.paramid, val);
            d.set_controlinfo(p.paramid, changeData.controlinfo());
            d.set_useinfo(p.paramid, changeData.useinfo());

            QModelIndex flagIndex = createIndex(index.row(), index.column() -1, 0);
            emit dataChanged(flagIndex, index);
            emit dataModification(changeData);
            qDebug() << "Station " << d.stnr() << " (" << d.name() << "): Changed parameter " << qPrintable(p.parameterName) << " from " << oldValue << " to " << val;
            return true;
          }
          catch ( InvalidIndex & ) {
              return false;
          }
      }
    }
    return false;
  }

  void KvalobsDataModel::setShowStationName(bool show)
  {
    if ( show != showStationNameInHeader_ ) {
      showStationNameInHeader_ = show;
      emit headerDataChanged(Qt::Vertical, 0, rowCount());
    }
  }

  void KvalobsDataModel::setShowPosition(bool show)
  {
    if ( show != showPositionInHeader_ ) {
        showPositionInHeader_ = show;
        emit headerDataChanged(Qt::Vertical, 0, rowCount());
    }
  }

  void KvalobsDataModel::setShowHeight(bool show)
  {
    if ( show != showHeightInHeader_ ) {
        showHeightInHeader_ = show;
        emit headerDataChanged(Qt::Vertical, 0, rowCount());
    }
  }

  QVariant KvalobsDataModel::displayRoleData(const QModelIndex & index) const
  {
    cerr.flags(ios::fixed);
    const Parameter & p = getParameter(index);

    const KvalobsData & d = kvalobsData_->at(index.row());
    const kvalobs::kvControlInfo & controlinfo = d.controlinfo(p.paramid);

    const int fmis = controlinfo.flag(kvalobs::flag::fmis);

    ColumnType columnType = getColumnType(index);
    switch ( columnType )
    {
    case Original:
      {
        if ( fmis == 1 or fmis == 3 )
          return QVariant();
	if ( paramIsCode(p.paramid) )
	  return int(d.orig(p.paramid));
	else
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
	//	double ccoor = d.corr(p.paramid);
	if ( paramIsCode(p.paramid) )
	  return int(d.corr(p.paramid));
	else
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

  QVariant KvalobsDataModel::textColorRoleData(const QModelIndex & index) const
  {
    ColumnType columnType = getColumnType(index);
    if ( Corrected == columnType ) {
        if ( index.isValid() and index.row() < (int)kvalobsData_->size() ) {
            const KvalobsData & d = kvalobsData_->at(index.row());
            const kvalobs::kvControlInfo & ci = d.controlinfo(getParameter(index).paramid);
            if ( ci.flag(kvalobs::flag::fhqc) == 0 ) { // not hqc touched
              if ( ci.qc2dDone() )
                return Qt::darkMagenta;
              if ( ci.flag(kvalobs::flag::fnum) >= 6 )
                return Qt::red;
            }
        }
    }
    return Qt::black;
  }

  QVariant KvalobsDataModel::textAlignmentRoleData(const QModelIndex & /*index*/) const
  {
    return Qt::AlignRight;
  }

  int KvalobsDataModel::dataColumn(QString parameter) const
  {
    int index = 2;
    mi_foreach(const Parameter& p, parametersToShow_) {
        if( parameter == p.parameterName ) {
            break;
        }
        index += COLUMNS_PER_PARAMETER;
    }
    return index;
  }
  int KvalobsDataModel::dataRow(int stationid, const timeutil::ptime& obstime, ObstimeMatch otm) const
  {
      LOG_FUNCTION();
      // FIXME this routine will probably cause segfault if rows == 0

      const KvalobsDataListPtr & data = kvalobsData();
      const int rows = data->size();
      int best_index = -1;
      timeutil::ptime best_time;
      int second_best_index = 0;
      for (int index=0; index<rows; ++index) {
          const KvalobsData & d = (*data)[index];
          if( d.stnr() == stationid ) {
              second_best_index = index;
              if (d.otime() == obstime) {
                  qDebug() << "exact obstime=" << QString::fromStdString(timeutil::to_iso_extended_string(obstime))
                           << "found";
                  return index;
              } else if( (otm == OBSTIME_BEFORE    and d.otime() <= obstime and (best_index < 0 or d.otime() > best_time))
                         or (otm == OBSTIME_AFTER  and d.otime() >= obstime and (best_index < 0 or d.otime() < best_time)) )
              {
                  best_time = d.otime();
                  best_index = index;
              }
          }
      }
      if( best_index>=0 ) {
          qDebug() << "relative obstime=" << QString::fromStdString(timeutil::to_iso_extended_string(best_time))
                   << "found";
          return best_index;
      } else {
          qDebug() << "no good obstime=" << QString::fromStdString(timeutil::to_iso_extended_string(best_time))
                   << "found";
          return second_best_index;
      }
  }

  const KvalobsDataModel::Parameter & KvalobsDataModel::getParameter(const QModelIndex &index) const
  {
    return getParameter(index.column());
  }

  const KvalobsDataModel::Parameter & KvalobsDataModel::getParameter(int column) const
  {
    return parametersToShow_.at(column / COLUMNS_PER_PARAMETER);
  }

  KvalobsDataModel::ColumnType KvalobsDataModel::getColumnType(const QModelIndex & index) const
  {
    return getColumnType(index.column());
  }

  KvalobsDataModel::ColumnType KvalobsDataModel::getColumnType(int column) const
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
    return Original; // not reached
  }

  kvalobs::kvData KvalobsDataModel::getKvData_(const QModelIndex & index) const
  {
      if ( not index.isValid() or index.row() >= (int)kvalobsData_->size() )
      throw InvalidIndex();

    const Parameter & parameter = getParameter(index);
    const KvalobsData & d = kvalobsData_->at(index.row());

    return d.getKvData(parameter.paramid);
  }

  bool KvalobsDataModel::paramIsCode(const int parNo) const {
    for ( int i = 0; i < 66; i++ ) {
      if ( parNo == codeParam[i] )
	return true;
    }
    return false;
  }
}
