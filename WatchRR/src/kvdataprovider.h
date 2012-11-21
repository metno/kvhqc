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
#ifndef __WatchRR__KvDataProvider_h__
#define __WatchRR__KvDataProvider_h__

#include <list>
#include <kvalobs/kvData.h>

namespace WatchRR
{
  /**
   * Will provide kvData objects. In the cell implementation the
   * returned objects will not reflect changes having been done to
   * them.
   */
  class KvDataProvider
  {
  public:
    typedef kvalobs::kvData *            value_type;
    typedef std::list<value_type>        Data;
    typedef Data::iterator               Iterator;
    typedef Data::const_iterator         CIterator;
    
    virtual ~KvDataProvider() {}
    
    virtual void getKvData( Data &dataList ) const =0;
    
    inline Data getKvData() const
    { 
      Data ret;
      getKvData( ret );
      return ret;
    }
  };
}

#endif // __WatchRR__KvDataProvider_h__
