/* -*- c++ -*-

HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id$

Copyright (C) 2007 met.no

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
#ifndef PARAMETERDIALOG_H
#define PARAMETERDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QListWidget>
#include <QtGui/QRadioButton>
#include <QtGui/QPushButton>

class ParameterDialog : public QDialog {
  Q_OBJECT

public:
  ParameterDialog(QWidget*);

  void hideAll();
  void showAll();

  /*!
   * \brief Inserts the parameters in a weather element
   *        into a listbox in the parameter dialog.
   */
  void insertParametersInListBox(const std::vector<int> & porder, const QMap<int,QString> & parMap);

  bool isSelectedParameter(int paramIndex) const;

private slots:
  void selectionChanged();

signals:
  void paramHide();
  void paramApply();

public:
  // TODO these things should not be public
  QRadioButton* allPar;
  QRadioButton* markPar;
  QRadioButton* noMarkPar;

  QListWidget* plb;
};

#endif
