/* -*- c++ -*-

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

#ifndef UTIL_GUI_BUSYINDICATOR_HH
#define UTIL_GUI_BUSYINDICATOR_HH

#include <qglobal.h>

QT_BEGIN_NAMESPACE
class QMainWindow;
class QString;
class QWidget;
QT_END_NAMESPACE

// ------------------------------------------------------------------------

/*! Show a busy or wait indicator during the lifetime of this object.
 */
class BusyIndicator
{
public:
  BusyIndicator(bool wait=true);
  ~BusyIndicator();
};

// ------------------------------------------------------------------------

/*! Disable the GUI during the lifetime of this object.
 *
 * This may be pretty slow as all the GUI has to be redrawn, in
 * particular tables seem to query the table model for all visible
 * cells.
 */
class DisableGUI {
public:
    DisableGUI(QWidget* widget);
    ~DisableGUI();
protected:
    QWidget* mWidget;
    bool mWasEnabled;
};

// ------------------------------------------------------------------------

/*! Show a wait or busy cursor, a status message, and disable the GUI
 *  for the lifetime of this object.
 */
class BusyStatus : public DisableGUI, public BusyIndicator
{
public:
  BusyStatus(QMainWindow* mw, const QString& message, bool wait=true);
  ~BusyStatus();
};

#endif // UTIL_GUI_BUSYINDICATOR_HH
