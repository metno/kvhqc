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

#include "KvalobsDataView.h"
#include "KvalobsDataModel.h"
#include "KvalobsDataDelegate.h"
#include <QMessageBox>
#include <QLineEdit>
#include <QDebug>
#include <stdexcept>

namespace model
{
  KvalobsDataView::~KvalobsDataView()
  {
  }

  void KvalobsDataView::toggleShowFlags(bool show)
  {
    const KvalobsDataModel * model = getModel_();
    if ( model ) {
        int columns = model->columnCount();
        for ( int i = 0; i < columns; ++ i)
          if ( model->getColumnType(i) == KvalobsDataModel::Flag )
            setColumnHidden (i, not show);
    }
  }

  void KvalobsDataView::toggleShowOriginal(bool show)
  {
    const KvalobsDataModel * model = getModel_();
    if ( model ) {
        int columns = model->columnCount();
        for ( int i = 0; i < columns; ++ i)
          if ( model->getColumnType(i) == KvalobsDataModel::Original )
            setColumnHidden (i, not show);
    }
  }

  void KvalobsDataView::toggleShowModelData(bool show)
  {
    const KvalobsDataModel * model = getModel_();
    if ( model ) {
      int columns = model->columnCount();
      for ( int i = 0; i < columns; ++ i) {
        if ( model->getColumnType(i) == KvalobsDataModel::Model ) {
            if ( modelParameters_.find(model->getParameter(i).paramid) == modelParameters_.end() )
              setColumnHidden (i, true);
            else
              setColumnHidden (i, not show);
        }
      }
    }
  }


  void KvalobsDataView::selectStation(const QString & station)
  {
    QStringList elements = station.split(',');
    int elementCount = elements.size();
    if ( elementCount == 1 ) {
        miutil::miTime obstime(elements[0].toStdString());
        if ( obstime.undef() ) {
            qDebug() << "Unable to parse second element as obstime: " << station;
            return;
        }
        selectTime(obstime);
    }
    else if ( elementCount == 2 ) {
      bool ok;
      int stationid = elements[0].toInt(& ok);
      if ( not ok ) {
          qDebug() << "Unable to parse first element as stationid: " << station;
          return;
      }
      miutil::miTime obstime(elements[1].toStdString());
      if ( obstime.undef() ) {
          qDebug() << "Unable to parse second element as obstime: " << station;
          return;
      }
      selectStation(stationid, obstime);
    }
    else
      qDebug() << "Unable to parse station string: " << station;
  }

  class DebugPrint
  {
    const char * name_;
  public:
    DebugPrint(const char * name) : name_(name) {
      qDebug() << '+' << name_;
    }
    ~DebugPrint() {
      qDebug() << ' ' << name_;
    }
  };
#define LOG_FUNCTION() DebugPrint INTERNAL_function_logger(__func__)

  void KvalobsDataView::selectStation(int stationid, const miutil::miTime & obstime)
  {
    LOG_FUNCTION();

    const KvalobsDataModel * model = getModel_();
    if ( ! model )
      return;

    QModelIndex current = currentIndex();
    if ( not current.isValid() ) {
      current = model->index(0, 0);
      if ( not current.isValid() )
        return;
    }

    int row = model->dataRow(stationid, obstime);
    int column = current.column();
    QModelIndex index = model->index(row, column);

    blockSignals(true);
    if ( index.isValid() ) {
        setCurrentIndex(index);
    }
    else
      clearSelection();
    blockSignals(false);
  }

  void KvalobsDataView::selectTime(const miutil::miTime & obstime)
  {
    const KvalobsDataModel * model = getModel_();
    if ( ! model )
      return;

    const KvalobsData * data = model->kvalobsData(currentIndex());
    if ( data )
      selectStation(data->stnr(), obstime);
  }

  void KvalobsDataView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
  {
    LOG_FUNCTION();
    QTableView::currentChanged(current, previous);

    const KvalobsDataModel * model = getModel_();
    if ( model ) {
      if ( current.column() != previous.column() ) {
          try {
            QString parameterName = model->getParameter(current).parameterName;
            qDebug() << "parameterSelected(" << qPrintable(parameterName) << ")";
            emit parameterSelected(parameterName);
          }
          catch (std::out_of_range & ) {
              // invalid index - do not do anything
          }
      }

      const KvalobsData * oldData = model->kvalobsData(previous);
      const KvalobsData * newData = model->kvalobsData(current);

      if ( ! oldData )
        qDebug() << "Invalid oldData";
      if ( ! newData ) {
          qDebug() << "Invalid newData";
        return;
      }
      if ( (! oldData) or newData->stnr() != oldData->stnr() ) {
          qDebug() << "stationSelected(" << newData->stnr() << ")";
        emit stationSelected(newData->stnr());
      }
      if ( (! oldData) or newData->otime() != oldData->otime() ) {
          qDebug() << "timeSelected(" << newData->otime().isoTime().c_str() << ")";
        emit timeSelected(newData->otime());
      }
    }
  }

  const KvalobsDataModel * KvalobsDataView::getModel_() const
  {
    return dynamic_cast<const KvalobsDataModel *>(model());
  }

  void KvalobsDataView::setup_()
  {
    setItemDelegate(new KvalobsDataDelegate(this));
  }
}
