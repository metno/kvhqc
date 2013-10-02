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

#ifndef __CREATE_KV_COMPARISON_functors_h__
#define __CREATE_KV_COMPARISON_functors_h__

#ifdef CREATE_KV_EQUAL_COMPARISON
#error #CREATE_KV_EQUAL_COMPARISON already defined!
#endif
#ifdef CREATE_KV_LT_COMPARISON
#error #CREATE_KV_LT_COMPARISON already defined!
#endif


#define CREATE_KV_EQUAL_COMPARISON(PARAM) \
  struct equal_##PARAM { \
    template< typename A, typename B > \
    bool operator()( const A & a, const B & b ) const { \
      return a.PARAM() == b.PARAM(); \
    } \
  }

#define CREATE_KV_LT_COMPARISON(PARAM) \
  struct lt_##PARAM { \
    template< typename A, typename B > \
    bool operator()( const A & a, const B & b ) const { \
      return a.PARAM() < b.PARAM();	 \
    } \
  }


namespace kvalobs
{
	CREATE_KV_EQUAL_COMPARISON(stationID);
	CREATE_KV_EQUAL_COMPARISON(paramID);
	CREATE_KV_EQUAL_COMPARISON(typeID);
	CREATE_KV_EQUAL_COMPARISON(level);
	
	CREATE_KV_LT_COMPARISON(stationID);
	CREATE_KV_LT_COMPARISON(paramID);
	CREATE_KV_LT_COMPARISON(typeID);
	CREATE_KV_LT_COMPARISON(level);
}

#undef CREATE_KV_EQUAL_COMPARISON
#undef CREATE_KV_LT_COMPARISON

template< typename FunctorA, typename FunctorB >
struct combined {
  FunctorA fa;
  FunctorB fb;

  combined( ) : fa( ), fb( ) { }

  combined( FunctorA fa, FunctorB fb )
    : fa( fa ), fb( fb ) { }

  template< typename A, typename B >
  bool operator()( const A & a, const B & b ) const {
    return fa( a, b ) and fb( a, b );
  }
};

template< typename Param, typename Functor >
struct single_compare {
  const Param & p;
  Functor & func;
  single_compare( const Param & p, Functor & func )
    : p( p ), func( func ) { }


  template< typename OtherParam >
  bool operator()( const OtherParam & op ) const {
    return func( p, op );
  }
};

#endif // __CREATE_KV_COMPARISON_functors_h__
