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
#include "ExtendedFunctionalityHandler.h"
#include <qevent.h>


ExtendedFunctionalityHandler::
ExtendedFunctionalityHandler( ErrorList *el, QObject *parent, const char *name )
  : QObject( parent, name )
  , el(el), row(-1), function(0), key(Qt::Key_unknown)
{
  el->installEventFilter( this );
}

ExtendedFunctionalityHandler::
ExtendedFunctionalityHandler( ErrorList *el, int row, 
			      Function function,
			      Qt::Key key, QString explText, 
			      QObject *parent, const char *name )
  : QObject( parent, name )
  , el(el), row(row)
{
  el->installEventFilter( this );
  init( row, function, key, explText );
}

ExtendedFunctionalityHandler::
~ExtendedFunctionalityHandler( )
{
  // Causes segmentation fault when app closes:
  //clear();
}

void ExtendedFunctionalityHandler::
reset( int row, Function function, Qt::Key key, QString explText )
{
  clear();
  init( row, function, key, explText );
}

void ExtendedFunctionalityHandler::
init( int row, Function function, Qt::Key key, QString explText )
{
  this->row = row;
  this->function = function;
  this->key = key;
  el->setText( row, col, explText );
}

void ExtendedFunctionalityHandler::clear()
{
  if ( el )
    el->clearCell( row, col );
  key = (Qt::Key) 0;
}

bool ExtendedFunctionalityHandler::
eventFilter( QObject *watched, QEvent *e )
{
  if ( key == Qt::Key_unknown )
    return false;

  if ( e->type() != QEvent::KeyRelease )
    return QObject::eventFilter( watched, e );

  QKeyEvent *ke = dynamic_cast<QKeyEvent*>( e );

  cerr << "Key: " << ke->key() << " "
       << "(Wanted: " << key << ")\n";
  
  if ( ! ke or ke->key() != key or ! (ke->state() & Qt::ControlButton) )
    return QObject::eventFilter( watched, e );

  ErrorList *el = dynamic_cast<ErrorList*>( watched );
  if ( ! el ) 
    return false;

  function( el );
  return true;
}
