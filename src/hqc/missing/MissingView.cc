/*
 * MissingView.cc
 *
 *  Created on: Sep 26, 2013
 *      Author: vegardb
 */

#include "MissingView.hh"
#include <QTreeView>
#include <QBoxLayout>

MissingView::MissingView(QWidget* parent) :
    QWidget(parent)
{
  view = new QTreeView(this);

  view->setSortingEnabled(true);

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

