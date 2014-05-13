/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2014 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  HQC is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with HQC; if not, write to the Free Software Foundation Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "EditVersionsView.hh"

#include "EditVersionModel.hh"

#include <QBoxLayout>
#include <QEvent>
#include <QTreeView>
#include <QToolButton>

#define MILOGGER_CATEGORY "kvhqc.EditVersionsView"
#include "common/ObsLogging.hh"

EditVersionsView::EditVersionsView(EditVersionModel* model, QWidget* parent)
  : QWidget(parent)
{
  METLIBS_LOG_SCOPE();

  QVBoxLayout* vl = new QVBoxLayout(this);
  vl->setSpacing(2);
  vl->setContentsMargins(2, 2, 2, 2);

  QHBoxLayout* hl = new QHBoxLayout();

  mButtonUndo = new QToolButton(this);
  mButtonUndo->setIcon(QIcon("icons:undo.svg"));
  hl->addWidget(mButtonUndo);

  mButtonRedo = new QToolButton(this);
  mButtonRedo->setIcon(QIcon("icons:redo.svg"));
  hl->addWidget(mButtonRedo);

  mButtonSave = new QToolButton(this);
  hl->addWidget(mButtonSave);

  QSpacerItem* spacer = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  hl->addItem(spacer);

  vl->addLayout(hl);

  mTree = new QTreeView(this);
  vl->addWidget(mTree);

  retranslateUi();

  mTree->setModel(model);
  mTree->setSelectionBehavior(QTreeView::SelectRows);
  mTree->setSelectionMode(QTreeView::SingleSelection);

  connect(model->editAccess().get(), SIGNAL(currentVersionChanged(size_t, size_t)),
      this, SLOT(onEditVersionChanged(size_t, size_t)));
  connect(mButtonSave, SIGNAL(clicked()), this, SIGNAL(saveRequested()));
}

EditVersionsView::~EditVersionsView()
{
}

void EditVersionsView::onEditVersionChanged(size_t current, size_t highest)
{
  METLIBS_LOG_SCOPE();

  EditAccess_p eda = static_cast<EditVersionModel*>(mTree->model())->editAccess();
  METLIBS_LOG_DEBUG(LOGVAL(eda->countU()));
  mButtonSave->setEnabled(eda->canUndo());
  mButtonUndo->setEnabled(eda->canUndo());
  mButtonRedo->setEnabled(eda->canRedo());
  mTree->expandToDepth(2);
}

void EditVersionsView::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QWidget::changeEvent(event);
}

void EditVersionsView::retranslateUi()
{
  setWindowTitle(tr("Change History"));
  mButtonUndo->setText(tr("Undo"));
  mButtonRedo->setText(tr("Redo"));
  mButtonSave->setText(tr("Save"));
}
