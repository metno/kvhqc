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
#ifndef __ExtendedFunctionalityHandler_h__
#define __ExtendedFunctionalityHandler_h__

#include <errorlist.h>
#include <qobject.h>
//Added by qt3to4:
#include <QEvent>
#include <kvalobs/kvData.h>

/**
 * \brief Intercepts keypress \code key from the \code ErrorList, and
 * possibly calls \code function(el) on them. Also provides an
 * explanatory text in the rightmost row of the \code ErrorList.
 */
class ExtendedFunctionalityHandler 
  : public QObject
{
  Q_OBJECT;
  static const int col = 20;
public:

  typedef void ( *Function )( ErrorList *el );

  ExtendedFunctionalityHandler( ErrorList *el, 
				QObject *parent = 0, const char *name = 0 );

  ExtendedFunctionalityHandler( ErrorList *el, int row, 
				Function function,
				Qt::Key key, QString explText, 
				QObject *parent = 0, const char *name = 0 );
  virtual ~ExtendedFunctionalityHandler( );

  virtual void reset( int row, Function function, Qt::Key key, QString explText );

  virtual void clear();

protected:

  virtual void init( int row, Function function, Qt::Key key, QString explText );

  virtual bool eventFilter( QObject *watched, QEvent *e );

  ErrorList *el;
  int row;
  Function function;
  Qt::Key key;

};

#endif // __ExtendedFunctionalityHandler_h__
