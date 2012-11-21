/* -*- c++ -*-

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

#ifndef KVALOBSDATADELEGATE_H_
#define KVALOBSDATADELEGATE_H_

#include <QStyledItemDelegate>
#include <QMap>

class HqcMainWindow;
class QLineEdit;

namespace model
{

  class KvalobsDataDelegate : public QStyledItemDelegate
  {
    Q_OBJECT
  public:
    KvalobsDataDelegate(QObject * parent = 0);
    virtual ~KvalobsDataDelegate();

    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;

    void setEditorData(QWidget * editor, const QModelIndex & index) const;

    virtual void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;

    virtual void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const;

    virtual QString displayText(const QVariant& value, const QLocale& locale) const;

  private:
    HqcMainWindow * mainWindow;
    void setup_();

    typedef QLineEdit Editor;

    bool legalTime(int hour, int parameter, double value) const;
    bool legalValue(double value, int parameter) const;
    QMap<int, float> lowMap;
    QMap<int, float> highMap;
  };

}

#endif /* KVALOBSDATADELEGATE_H_ */
