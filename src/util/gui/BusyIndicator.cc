/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2013 met.no

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

#include "BusyIndicator.hh"

#include <QtGui/QApplication>
#include <QtGui/QCursor>
#include <QtGui/QMainWindow>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

BusyIndicator::BusyIndicator(bool wait)
{
    qApp->setOverrideCursor(wait ? Qt::WaitCursor : Qt::BusyCursor);
    qApp->processEvents();
}

BusyIndicator::~BusyIndicator()
{
    qApp->restoreOverrideCursor();
}

// ========================================================================

DisableGUI::DisableGUI(QWidget* widget)
    : mWidget(widget)
    , mWasEnabled(mWidget->isEnabled())
{
    if (mWasEnabled) {
        mWidget->setEnabled(false);
        qApp->processEvents();
    }
}
DisableGUI::~DisableGUI()
{
    if (mWasEnabled) {
        mWidget->setEnabled(true);
        qApp->processEvents();
    }
}

// ========================================================================

BusyStatus::BusyStatus(QMainWindow* mw, const QString& message, bool wait)
    : DisableGUI(mw)
    , BusyIndicator(wait)
{
    mw->statusBar()->showMessage(message);
    qApp->processEvents();
}

BusyStatus::~BusyStatus()
{
    static_cast<QMainWindow*>(mWidget)->statusBar()->clearMessage();
    qApp->processEvents();
}
