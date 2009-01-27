/*
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
#ifndef HELPTREE_H
#define HELPTREE_H

#include <q3mainwindow.h>
#include <q3textbrowser.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3PopupMenu>

class QComboBox;
class Q3PopupMenu;

class HelpTree : public Q3MainWindow
{
    Q_OBJECT
public:
    HelpTree( const QString& home_,  const QString& path, QWidget* parent = 0, const char *name=0 );
    ~HelpTree();

private slots:
    void setBackwardAvailable( bool );
    void setForwardAvailable( bool );

    void sourceChanged( const QString& );
    void openFile();
    void newWindow();
    void print();

    void pathSelected( const QString & );
    void histChosen( int );

private:
    void readHistory();

    Q3TextBrowser* browser;
    QComboBox *pathCombo;
    int backwardId, forwardId;
    QStringList history;
    QMap<int, QString> mHistory;
    Q3PopupMenu *hist;

};
#endif
