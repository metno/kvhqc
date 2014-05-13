/* -*- c++ -*-
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

#ifndef EDITVERSIONS_VIEW_H
#define EDITVERSIONS_VIEW_H

#include "common/EditAccess.hh"

#include <QWidget>

class EditVersionModel;
class QToolButton;
class QTreeView;

class EditVersionsView : public QWidget
{ Q_OBJECT;
public:
  EditVersionsView(EditVersionModel* model, QWidget* parent=0);
  ~EditVersionsView();

Q_SIGNALS:
  void saveRequested();

protected:
  virtual void changeEvent(QEvent *event);
 
private Q_SLOTS:
  void onEditVersionChanged(size_t current, size_t highest);

private:
  void retranslateUi();

private:
  QToolButton* mButtonUndo;
  QToolButton* mButtonRedo;
  QToolButton* mButtonSave;
  QTreeView* mTree;
};

#endif // ERRORLIST_H
