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
#include "VxKvDataTableItem.h"
#include "hqc_paths.hh"
#include <fstream>
#include <sstream>

#include <iostream>

using namespace std;
using namespace kvalobs;



namespace WatchRR
{
  QStringList VxKvDataTableItem::selections;

  const QString VxKvDataTableItem::VxExplFile =
      ::hqc::getPath(::hqc::CONFDIR) + "/VxExplanations.txt";

  const char *VxKvDataTableItem::VxSExpl[ 3 ] =
    {
      "lett ", "", "mye "
    };

  VxKvDataTableItem::VxParams VxKvDataTableItem::vxparams;


  VxKvDataTableItem::VxKvDataTableItem( Q3Table * table, KvDataProvider::Data data_ )
      : RRComboTableItem( table, data_, selections )
  {
    if ( selections.isEmpty() )
    {
      selections.append("");

      //      ifstream fs( VxExplFile.c_str() );
      ifstream fs( VxExplFile.toStdString().c_str() );

      VxParam param;
      while ( fs >> param )
      {
        if ( ! param.kvalobsCode )
          continue;
        vxparams.insert( VxParams::value_type(param.metCode, param) );
        selections.append( param.metCode + " " );
        selections.append( param.metCode + QChar(Low) );
        selections.append( param.metCode + QChar(High) );
      }
    }

    setText( getSymbol( data_ ) );
  }
  
  QString VxKvDataTableItem::getSymbol( KvDataProvider::Data data_ ) const
  {
    if ( data_.empty() )
      return "";
    
    KvDataProvider::CIterator d = data_.begin();
    
    if ( not kvalobs::valid( ** d ) )
      return "";
        
    if ( kvalobs::valid( ** d ) )
    {
      for ( VxParams::const_iterator it = vxparams.begin(); it != vxparams.end(); it++ )
      {
        if ( (int) it->second.kvalobsCode == (int) (*d)->corrected() )
        {
          QString ret = it->second.metCode;
          if ( kvalobs::valid( ** ( ++ d ) ) )
          {
            if ( (*d)->corrected() == 0 )
              ret += QChar( Low );
            else if ( (*d)->corrected() == 2 )
              ret += QChar( High );
            else
              ret += " ";
          }
          else
            ret += " ";
          
          return ret;
        }
      }
    }
    
    return "Err";
  }


  VxKvDataTableItem::~VxKvDataTableItem()
  {
  }


  QString VxKvDataTableItem::explain( ) const
  {
    QString base = text();
    
    if ( base == "Err" )
      return QString( "Forstår ikke verdien i dette feltet" );

    if ( not has_valid_parameter_list() )
      //      return QString("Feil! Finner ikke ") + QString::fromStdString(VxExplFile) + "!";
      return QString("Feil! Finner ikke ") + VxExplFile + "!";

    QString ret = "Ingen data";

    if ( base.length() )
    {
      QString param = base.left( base.length() -1 );

      VxParams::const_iterator item = vxparams.find( param );

      if ( item == vxparams.end() )
        return "Ingen forklaring tilgjengelig";

      ret = item->second.explanation;
      QChar degree = base.at( base.length() -1 );
      if ( degree == QChar(Low) )
        ret = VxSExpl[0] + ret; //ret += VxSExpl[0];
      else if ( degree == QChar(High) )
        ret = VxSExpl[2] + ret;//ret += VxSExpl[2];
      ret[0] = ret[0].upper();
    }

    if ( not data.empty() )
    {
      ret += " (original fra stasjon: ";
      const kvData * Vx = data.front();
      if ( not kvalobs::original_missing( * Vx ) )
      {
        for ( VxParams::const_iterator it = vxparams.begin(); it != vxparams.end(); it++ )
        {
          if ( it->second.kvalobsCode == (int) Vx->original() )
          {
            ret += it->second.explanation;
            break;
          }
        }
      }
      else
        ret += "manglende";
      ret += ")";
    }

    return ret;
  }

  inline float getVal_( const kvData * data )
  {
    if ( kvalobs::valid( * data ) )
      return data->corrected();
    return CellValueProvider::missing;
  }

  void VxKvDataTableItem::getCellValue( CellValueProvider::Data &cellList ) const
  {
    if ( not has_valid_parameter_list() or had_invalid_value_and_not_modified() )
    {
      // Return the original values acquired from kvalobs
      KvDataProvider::Data dataList;
      getKvData( dataList );
      KvDataProvider::CIterator data = dataList.begin();
      cellList.push_back( getVal_( * data ) );
      data++;
      cellList.push_back( getVal_( * data ) );
      return;
    }

    QString base = text();

    float primary = CellValueProvider::missing;
    float secondary = CellValueProvider::missing;

    if ( not base.stripWhiteSpace().isEmpty() )
    {

      QString param = base.left( base.length() -1 );
      VxParams::const_iterator item = vxparams.find( param );

      if ( item != vxparams.end() )
      {
        primary = item->second.kvalobsCode;
        QChar sec = base[ base.length() -1 ];
        if ( sec == QChar( Low ) )
          secondary = 0;
        else if ( sec == QChar( High ) )
          secondary = 2;
        else if ( primary != CellValueProvider::missing )
          secondary = 1;
      }
    }
    cellList.push_back( primary );
    cellList.push_back( secondary );
  }


  bool VxKvDataTableItem::has_valid_parameter_list() const
  {
    return selections.size() > 1;
  }
  
  bool VxKvDataTableItem::had_invalid_value_and_not_modified() const
  {
    return text() == "Err";
  }

  bool VxKvDataTableItem::equal( const kvalobs::kvData & d, float v ) const
  {
    int p = d.paramID();
    if ( p == 35 or p == 37 or p == 39 )
      if ( not kvalobs::valid( d ) and v == 1 )
        return true;
    return RRComboTableItem::equal( d, v );
  }

  istream &operator>>( istream &s, VxKvDataTableItem::VxParam &p )
  {
    string oneLine;
    getline( s, oneLine );
    istringstream ss( oneLine );

    p.kvalobsCode = 0;

    if ( !ss ) return s;
    ss >> p.kvalobsCode;
    if ( p.kvalobsCode )
    {
      if ( !ss ) return s;
      string tmp;
      ss >> tmp;
      p.metCode = tmp.c_str();
    }
    if ( !ss ) return s;
    string tmp;
    getline( ss, tmp );
    p.explanation = tmp.c_str();
    p.explanation = p.explanation.stripWhiteSpace();

    return s;
  }
}
