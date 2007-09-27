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
#ifndef __WatchRR__RRTableItem_h__
#define __WatchRR__RRTableItem_h__

#include <qtable.h>
#include <qcombobox.h>
#include <selfexplainable.h>
#include <qvalidator.h>
#include "dataconsistencyverifier.h"

namespace WatchRR
{
  class RRTable;

  class RRTableItem
    : public QTableItem
    , public SelfExplainable
    , public DataConsistencyVerifier
  {
  public:
    //RRTableItem( QTable * table, int flags = 0, const QRegExpValidator * validator = 0 );
    RRTableItem( QTable * table, KvDataProvider::Data & data, const QRegExpValidator * validator = 0 );
    RRTableItem( QTable * table, kvalobs::kvData & data, const QRegExpValidator * validator = 0 );
    RRTableItem( QTable * table ); // Not editable

    virtual ~RRTableItem( );
    
    virtual QString explain() const;
    
    virtual void getCellValue( CellValueProvider::Data & cellList ) const;
    
    virtual void getKvData( KvDataProvider::Data & dataList ) const 
    { dataList.insert( dataList.end(), data.begin(), data.end() ); }
    
    bool readOnly() const { return data.empty(); }

    virtual int alignment() const { return Qt::AlignRight | Qt::AlignVCenter; }

    virtual QWidget * createEditor() const;
    
    virtual void setContentFromEditor( QWidget * w );

    virtual void paint( QPainter * p, const QColorGroup & cg, 
			const QRect & cr, bool selected );
      
    RRTable * rrTable();
    const RRTable * rrTable() const;

    using DataConsistencyVerifier::getKvData;
    using DataConsistencyVerifier::getCellValue;

  protected:
    KvDataProvider::Data data;
    const QRegExpValidator * validator;
    bool modelValue_;
    
    virtual QString getText() const;
    
    virtual QString getText( float f ) const;

    /**
     * \brief Identifies missing data.
     */
    static const QString missingDataIdentifier;

    virtual QColorGroup getColorGroup( const QColorGroup & base ) const;            
    
    RRTableItem( QTable * table, KvDataProvider::Data & data, QTableItem::EditType et, const QRegExpValidator * validator = 0 );
    RRTableItem( QTable * table, kvalobs::kvData & data, QTableItem::EditType et, const QRegExpValidator * validator = 0 );    
  };


  class RRComboTableItem
    : public RRTableItem
  {
  public:
    template<typename Data>
    RRComboTableItem( QTable * table, Data & data, QStringList & values )
      : RRTableItem( table, data, WhenCurrent )
      , values_( & values )
    {
    }
    
    RRComboTableItem( QTable * table );
  
    virtual QWidget * createEditor() const;
    
    virtual void setContentFromEditor( QWidget * w );
    
    void setValues( const QStringList * values ) { values_ = values; }
    
  protected:
    const QStringList * values_;
  };
  
  
  class RRCheckTableItem2
    : public QCheckTableItem
    , public DataConsistencyVerifier
    , public SelfExplainable
  {
  public:
    RRCheckTableItem2( QTable * table, KvDataProvider::Data & data_ )
      : QCheckTableItem( table, "")
      , data( data_ )
    {
      setReplaceable( false );
    }

    RRCheckTableItem2( QTable * table, kvalobs::kvData & data_ )
      : QCheckTableItem( table, "")
    {
      data.push_back( & data_ );
      setReplaceable( false );
    }
    
    virtual void getUpdatedList( DataSet & data ) = 0;
    
    virtual void getCellValue( CellValueProvider::Data & cellList ) const
    { cellList.push_back( isChecked() ? 1 : 0 ); }
    
    virtual void getKvData( KvDataProvider::Data & dataList ) const 
    { dataList.insert( dataList.end(), data.begin(), data.end() ); }
    
    using DataConsistencyVerifier::getKvData;
    using DataConsistencyVerifier::getCellValue;
    
  protected:
    KvDataProvider::Data data;
  };

}
#endif // __WatchRR__RRTableItem_h__
