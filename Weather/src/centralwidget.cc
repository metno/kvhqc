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
#include "centralwidget.h"
#include <qtabwidget.h>
#include <q3table.h>
#include <qlayout.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3GridLayout>



WeatherCentralWidget::WeatherCentralWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    mainGrid = new Q3GridLayout( this, 2, 1, 5, 5 );

    setupTabWidget();

    mainGrid->setRowStretch( 0, 0 );
    mainGrid->setRowStretch( 1, 1 );
}

void WeatherCentralWidget::setupTabWidget() {
    wTab = new QTabWidget(this);
    QWidget *corr = new QWidget( wTab );
    Q3GridLayout *grid1 = new Q3GridLayout( corr, 2, 5, 5, 5 );
    Q3Table* corrTab = new Q3Table(17, 17, corr);
    corrTab->resize( corrTab->sizeHint() );
    grid1->addWidget( corrTab, 1, 3 );
    wTab->addTab(corr, "Korrigert");

    QWidget *orig = new QWidget( wTab );
    Q3GridLayout *grid2 = new Q3GridLayout( orig, 2, 5, 5, 5 );
    Q3Table* origTab = new Q3Table(17, 17, orig);
    origTab->resize( origTab->sizeHint() );
    grid2->addWidget( origTab, 1, 3 );
    wTab->addTab(orig, "Original");

    QWidget *flag = new QWidget( wTab );
    Q3GridLayout *grid3 = new Q3GridLayout( flag, 2, 5, 5, 5 );
    Q3Table* flagTab = new Q3Table(17, 17, flag);
    flagTab->resize( flagTab->sizeHint() );
    grid3->addWidget( flagTab, 1, 3 );
    wTab->addTab(flag, "Flagg");
}
