/* -*- c++ -*-

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
#ifndef __WatchRR__CellValueProvider_h__
#define __WatchRR__CellValueProvider_h__

#include <limits>

#include <list>

namespace WatchRR
{
  /**
   * Will provide the contents of a cell, as a series of float
   * values. Meant to be used by modifiable QTable cells
   * representing kvData objects.
   */
  class CellValueProvider
  {
  public:
    typedef float                        value_type;
    typedef std::list<value_type>        Data;
    typedef Data::iterator               Iterator;
    typedef Data::const_iterator         CIterator;
    
    static const float missing;
    
	virtual ~CellValueProvider() {}
    
    virtual void getCellValue( Data &cellList ) const = 0;
    
    /**
     * A class must say the following in order to provide this method:
     * @code using CellValueProvider::getCellValue;
     */ 
    inline Data getCellValue() const
    {
      Data ret;
      getCellValue( ret );
      return ret;
    }
  };  
}

#endif // __WatchRR__CellValueProvider_h__
