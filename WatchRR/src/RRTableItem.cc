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
#include "RRTableItem.h"
#include "RRTable.h"
#include <qlineedit.h>

using namespace kvalobs;

namespace WatchRR
{
  const QString RRTableItem::missingDataIdentifier = "-";
  
  bool modelvalue_( const kvData * data )
  {
    return kvalobs::has_model_value( * data ) and ! data->controlinfo().flag( kvalobs::flag::fhqc );
  }
  
  bool modelvalue_( KvDataProvider::Data & data )
  {
    for ( KvDataProvider::CIterator it = data.begin(); it != data.end(); ++ it ) {
      if ( modelvalue_( * it ) )
        return true;
    }
    return false;
  }
  
  RRTableItem::RRTableItem( QTable * table, KvDataProvider::Data & data_, const QRegExpValidator * validator )
    : QTableItem( table, OnTyping )
    , data( data_ )
    , validator( validator )
    , modelValue_( modelvalue_( data_ ) )
  {
    setReplaceable( false );
  }

  RRTableItem::RRTableItem( QTable * table, kvalobs::kvData & data_, const QRegExpValidator * validator )
    : QTableItem( table, OnTyping )
    , validator( validator )
    , modelValue_( modelvalue_( & data_ ) )
  {
    data.push_back( & data_ );
    setReplaceable( false );
  }
  
  RRTableItem::RRTableItem( QTable * table, KvDataProvider::Data & data_, QTableItem::EditType et, const QRegExpValidator * validator )
    : QTableItem( table, et )
    , data( data_ )
    , validator( validator )
    , modelValue_( modelvalue_( data_ ) )
  {
    setReplaceable( false );
  }
  
  RRTableItem::RRTableItem( QTable * table, kvalobs::kvData & data_, QTableItem::EditType et, const QRegExpValidator * validator )
    : QTableItem( table, et )
    , validator( validator )
    , modelValue_( modelvalue_( & data_ ) )
  {
    data.push_back( & data_ );
    setReplaceable( false );
  }
  
    
  RRTableItem::RRTableItem( QTable * table )
    : QTableItem( table, Never )
    , validator( 0 )
    , modelValue_( false )
  {
  }

  
  RRTableItem::~RRTableItem( )
  {
  }

  QString RRTableItem::explain() const
  {
    QString ret = text();
    if ( modelValue_ )
      ret += " (modellverdi)";
    return ret;
  }
  
  void RRTableItem::getCellValue( CellValueProvider::Data & cellList ) const
  {
    bool ok;
    QString txt = text().stripWhiteSpace();
    float f = txt.toFloat( & ok );
    if ( ok )
      cellList.push_back( f );
    else if ( txt == missingDataIdentifier )
      cellList.push_back( CellValueProvider::missing );
      
    // else: subclasses must implement other cases.
  }

  QWidget * RRTableItem::createEditor() const
  {
    if ( data.empty() )
      return 0;
    QWidget * ret = QTableItem::createEditor();
    QLineEdit *le = dynamic_cast<QLineEdit *>( ret );
    if ( le ) {
      le->clear();
      le->setValidator( validator );
    }
    return ret;
  }
   
  void 
  RRTableItem::
  setContentFromEditor( QWidget * w )
  {
    QLineEdit * le = dynamic_cast<QLineEdit *>( w );
    if ( ! le )
      return; // Should never happen

    QString newText = le->text().stripWhiteSpace();
    newText = newText.replace( ',', '.' );
    
    QString originalText = getText();
    
    if ( newText.isEmpty() )
      newText = originalText;
      
    if ( newText == originalText )
      modelValue_ = modelvalue_( data );
    else
      modelValue_ = false;
      
    le->setText( newText );
            
    QTableItem::setContentFromEditor( w );
  }

  void RRTableItem::paint( QPainter * p, const QColorGroup & cg, 
			   const QRect & cr, bool selected )
  {
    QColorGroup color = getColorGroup( cg );
    QTableItem::paint( p, color, cr, selected );
  }

  QString RRTableItem::getText() const
  {
    if ( data.empty() )
      return missingDataIdentifier;
    const kvData * d = data.front();
    if ( not kvalobs::valid( * d ) )
      return missingDataIdentifier;
    return getText( d->corrected() );
  }
  
  QString RRTableItem::getText( float f ) const
  {
    if ( f == CellValueProvider::missing )
      return missingDataIdentifier;
    return QString::number( f, 'f', 1 );
  }

  QColorGroup RRTableItem::getColorGroup( const QColorGroup & base ) const
  {
    QColorGroup ret( base );
    if ( readOnly() )
      ret.setColor( QColorGroup::Base, QColor( 0, 0, 248, QColor::Hsv ) );
    if ( modelValue_ )
      ret.setColor( QColorGroup::Text, QColor( 255, 0, 0, QColor::Rgb ) );
    return ret;
  }
  
  RRTable * RRTableItem::rrTable() 
  {
    return dynamic_cast<RRTable *>( table() ); 
  }
  
  const RRTable * RRTableItem::rrTable() const 
  {
    return dynamic_cast< const RRTable *>( table() ); 
  }
  
  RRComboTableItem::RRComboTableItem( QTable * table )
    : RRTableItem( table )
  {
  }

  QWidget * RRComboTableItem::createEditor() const
  {
    if ( data.empty() )
      return 0;
    QComboBox * ret = new QComboBox( table()->viewport() );

    ret->insertStringList( * values_, 0 );
    ret->setCurrentText( text() );

    return ret;      
  }
  
  void RRComboTableItem::setContentFromEditor( QWidget * w )
  {
    QComboBox * cell = dynamic_cast<QComboBox *>( w );
    assert( cell );
    setText( cell->currentText() );
  }
  
}
