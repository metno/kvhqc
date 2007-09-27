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
#include "ControlFlagCell.h"
#include <sstream>

namespace WatchRR
{
  namespace cell
  {
    static QString getFullText( const kvalobs::kvControlInfo & cInfo );
    static QString getAbbrevText( const kvalobs::kvControlInfo & cInfo );

    ControlFlagCell::ControlFlagCell( QTable * t, const kvalobs::kvData & data )
      : RRTableItem( t )
      , data( data )
    {
      recalculateTexts();
    }
    
    ControlFlagCell::~ControlFlagCell( )
    {
    }

    static const char * flags[16] = {
      "fqclevel", "fr", "fcc", "fs", "fnum", 
      "fpos", "fmis", "ftime", "fw", "fstat", 
      "fcp", "fclim", "fd", "pre", "", "fhqc" 
    };

    QString ControlFlagCell::explain() const
    {
      return fullText();
    }

    void ControlFlagCell::recalculateTexts()
    {
      fullText_ = getFullText( data.controlinfo() );
      abbrevText_ = getAbbrevText( data.controlinfo() );
      setText();      
    }

    void ControlFlagCell::setText()
    {
      setText( fullText() );
    }

    static QString getFullText( const kvalobs::kvControlInfo & cInfo )
    {
      std::ostringstream ss;
      int f;
      for ( f = 1; f < 16; f++ ) {
	int flag = cInfo.flag( f );
	if ( flag != 0 ) {
	  ss << flags[f] << '=' << flag;
	  break;
	}
      }
      while ( ++f < 16 ) {
	int flag = cInfo.flag( f );
	if ( flag != 0 )
	  ss << " " << flags[f] << '=' << flag;
      }
      return ss.str();
    }

    static QString getAbbrevText( const kvalobs::kvControlInfo &  )
    {
      return "";
    }
  }
}
