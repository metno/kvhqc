/* -*- c++ -*-

HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id: dianashowdialog.h 1055 2011-04-12 10:53:17Z knutj $

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
#ifndef DISCARDDIALOG_H
#define DISCARDDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QListWidget>

class DiscardDialog : public QDialog
{
  Q_OBJECT
public:
  DiscardDialog( std::vector<QString>& ch );

  QListWidget* resultWidget;

  QPushButton* okButton;
  QPushButton* cancelButton;

signals:
  void tsReject();
  void tsCancel();
};

#endif
