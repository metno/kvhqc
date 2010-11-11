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
#include <QDebug>

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

  const KvalobsDataModel * KvalobsDataView::getModel_() const
  {
    return dynamic_cast<const KvalobsDataModel *>(model());
  }

}
