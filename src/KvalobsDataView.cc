/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2013 met.no

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

#include "hqc_paths.hh"
#include "KvalobsDataModel.h"
#include "KvalobsDataDelegate.h"
#include "KvMetaDataBuffer.hh"

#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>

#include <stdexcept>

#define NDEBUG
#include "debug.hh"

namespace model
{
KvalobsDataView::KvalobsDataView(QWidget* parent)
    : QTableView(parent)
{
    setup_();
}

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
    if (not model)
        return;

    const int columns = model->columnCount();
    for (int i=0; i<columns; ++i) {
        if (model->getColumnType(i) != KvalobsDataModel::Model)
            continue;
        const bool showParam
            = show and (KvMetaDataBuffer::instance()->isModelParam(model->getParameter(i).paramid));
        setColumnHidden(i, not showParam);
    }
}

void KvalobsDataView::selectStation(int stationid, const timeutil::ptime& obstime, int typeID)
  {
    LOG_SCOPE();
    DBG(DBG1(stationid) << DBG1(obstime) << DBG1(typeID));

    const KvalobsDataModel * model = getModel_();
    if ( ! model )
      return;

    QModelIndex current = currentIndex();
    if ( not current.isValid() ) {
      current = model->index(0, 0);
      if ( not current.isValid() )
        return;
    }

    int row = model->dataRow(stationid, obstime, KvalobsDataModel::OBSTIME_EXACT, typeID);
    int column = current.column();
    // UNUSED int currow = current.row();
    QModelIndex index = model->index(row, column);

    blockSignals(true);
    if ( index.isValid() ) {
        setCurrentIndex(index);
        selectRow(row);
    } else {
        clearSelection();
    }
    blockSignals(false);
  }

  void KvalobsDataView::selectTime(const timeutil::ptime& obstime)
  {
    const KvalobsDataModel * model = getModel_();
    if ( ! model )
      return;

    const KvalobsData * data = model->kvalobsData(currentIndex());
    if ( data )
        selectStation(data->stnr(), obstime, data->showTypeId());
  }

void KvalobsDataView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    LOG_SCOPE();
    QTableView::currentChanged(current, previous);

    if (not current.isValid())
        return;
    
    const KvalobsDataModel * model = getModel_();
    if (not model)
        return;

    try {
        /*emit*/ signalNavigateTo(model->getKvData_(current));
    } catch (std::exception&) {
    }
}

const KvalobsDataModel * KvalobsDataView::getModel_() const
{
    return dynamic_cast<const KvalobsDataModel *>(model());
}

void KvalobsDataView::setup_()
{
    setAttribute(Qt::WA_DeleteOnClose);
    const QString hqc_icon_path = ::hqc::getPath(::hqc::IMAGEDIR) + "/hqc.png";
    setIcon(QPixmap(hqc_icon_path));
    setCaption(tr("Data List"));
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    verticalHeader()->setDefaultSectionSize(20);
    horizontalHeader()->setMinimumSectionSize(20);
    horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    setItemDelegate(new KvalobsDataDelegate(this));
}
} // namespace model
