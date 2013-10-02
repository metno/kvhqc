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

  QHBoxLayout * mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(view);
}

MissingView::~MissingView()
{
}


