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
#include "common/HqcApplication.hh"

#include <QHeaderView>

#define MILOGGER_CATEGORY "kvhqc.EditVersionsView"
#include "common/ObsLogging.hh"

EditVersionsView::EditVersionsView(EditAccess_p eda, QWidget* parent)
  : QTreeView(parent)
{
  EditVersionModel* mdl = new EditVersionModel(hqcApp->editAccess(), this);
  setModel(mdl);

  header()->setResizeMode(QHeaderView::Interactive);
  header()->resizeSection(EditVersionModel::COL_TIME, 200);
  header()->resizeSection(EditVersionModel::COL_STATION, 50);
  header()->resizeSection(EditVersionModel::COL_SENSORNR, 30);
  header()->resizeSection(EditVersionModel::COL_LEVEL, 30);
  header()->resizeSection(EditVersionModel::COL_TYPEID, 40);
  header()->resizeSection(EditVersionModel::COL_PARAMID, 40);
  header()->resizeSection(EditVersionModel::COL_CORRECTED, 50);
  header()->resizeSection(EditVersionModel::COL_FLAGS, 120);

  setSelectionBehavior(QTreeView::SelectRows);
  setSelectionMode(QTreeView::SingleSelection);

  connect(eda.get(), SIGNAL(currentVersionChanged(size_t, size_t)),
      this, SLOT(onEditVersionChanged(size_t, size_t)));
}

EditVersionsView::~EditVersionsView()
{
}

void EditVersionsView::onEditVersionChanged(size_t current, size_t highest)
{
  expandToDepth(2);
}
