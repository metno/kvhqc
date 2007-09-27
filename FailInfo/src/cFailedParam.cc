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
#include "cFailedParam.h"

#include <iostream>

using namespace std;

namespace QC
{

  cFailedParam::cFailedParam( const string &param )
    : cfailed( param )
  {
    int start = 0;
    int end;
    for ( int i = 0; i < 3; i++ ) {
      indexes[i].start = start;
      end = cfailed.find( '-', start );
      indexes[i].length = end - start;
      start = end +1;
    }

    // Remove a tail of type ":1" from Detail:
    string::size_type pos = param.find( ':', indexes[2].start );
    if ( pos != string::npos )
      indexes[2].length = pos - indexes[2].start;
  }
  
  
  cFailedParam::~cFailedParam( )
  {
  }
  
  
  string cFailedParam::getPart( ParamPart part ) const
  {
    const Index &index = indexes[ part ];
    return cfailed.substr( index.start, index.length );
  }


  bool cFailedParam::altLess::operator()( const cFailedParam &main, const cFailedParam &other )
  { 
    // Why doesn't this work?
    // return main.toString().compare( other.toString() ) < 0;

    /*
    std::string a = main.toString();
    std::string b = other.toString();
    int len = std::min( a.size(), b.size() );
    
    for ( int pos = 0; pos < len; pos++ )
      if ( a[ pos ] < b[ pos ] )
	return true;
    return false;
    */

    string a =  main.toString();
    string b = other.toString();
    
    string::size_type apos = a.find(':');
    string::size_type bpos = b.find(':');

    return a.compare( 0, apos, b, 0, bpos ) < 0;
  }


  cFailList getFailList( const std::string &cfailed )
  {
    cFailList ret;
    string::size_type begin = 0;
    string::size_type end = 0;
    while ( end != string::npos ) {
      end = cfailed.find( ',', begin );
      cFailedParam fpar( cfailed.substr( begin, end - begin ) );
      ret.push_back( fpar );
      begin = end;
      begin++;
    }
    return ret;
  }
}

ostream &operator<<( ostream &stream, 
		     const QC::cFailList &failList )
{
  if ( not failList.empty() ) {
    QC::cFailList::const_iterator it = failList.begin(); 
    stream << *it;
    while ( ++it != failList.end() )
      stream << ',' << *it;
  }
  return stream;
}

