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
#include "FailDialog.h"
#include <qlayout.h>
//Added by qt3to4:
#include <Qt3Support/Q3HBoxLayout>
#include <Qt3Support/Q3VBoxLayout>

namespace FailInfo {

  FailDialog::FailDialog( QWidget *parent, const char *name, Qt::WFlags f )
    : QDialog( parent, name, false, f )
  {
    setCaption( tr("Feilinformasjon") );
    failList =   new FailList( this );
    hideButton = new QPushButton( tr("&Skjul"), this, "HideButton" );

    connect( hideButton, SIGNAL( clicked() ),
	     this,       SLOT  ( close()   ) );
    connect( parent, SIGNAL( errorListClosed() ),
	     this,   SLOT  ( close()   ) );

    Q3VBoxLayout *mainLayout = new Q3VBoxLayout( this, 2, 2 );
    mainLayout->addWidget(failList);
    
    Q3HBoxLayout *buttonLayout = new Q3HBoxLayout( mainLayout );
    buttonLayout->insertStretch( 0 );
    buttonLayout->addWidget( hideButton );
  }

  FailDialog::~FailDialog()
  {
  }
}
