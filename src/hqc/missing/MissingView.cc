/*
 * MissingView.cc
 *
 *  Created on: Sep 26, 2013
 *      Author: vegardb
 */

#include "MissingView.hh"
#include <common/Sensor.hh>
#include <QTreeView>
#include <QBoxLayout>
#include <QStandardItemModel>
#include <QDate>


MissingView::MissingView(QWidget* parent) :
    QWidget(parent)
{
  view = new QTreeView(this);

  view->setSortingEnabled(true);
  view->setEditTriggers(QAbstractItemView::NoEditTriggers);

  connect(view, SIGNAL(activated(QModelIndex)), SLOT(signalSelected(QModelIndex)));

  QHBoxLayout * mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(view);
}

MissingView::~MissingView()
{
}

void MissingView::setModel(QAbstractItemModel * model)
{
    view->setModel(model);
}

void MissingView::signalSelected(const QModelIndex & index)
{
    QStandardItemModel * model = dynamic_cast<QStandardItemModel *>(view->model());

    int stationId = model->item(index.row(), 0)->text().toInt();
    int typeId = 302; // TODO: fix this
    const QDate & qdate = model->item(index.row(), 2)->data().toDate();
    boost::gregorian::date date(qdate.year(), qdate.month(), qdate.day());

    Sensor sensor(stationId, 110, 0, 0, typeId);

    SensorTime st(sensor, boost::posix_time::ptime(date, boost::posix_time::hours(6)));
    selected(st);
}
