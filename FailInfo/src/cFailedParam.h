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
#ifndef __cFailedParam_h__
#define __cFailedParam_h__

#include <string>
#include <ostream>
#include <list>
#include <kvalobs/kvData.h>

namespace QC
{
  /**
   * \brief Represents a single QC check (such as QC1-2-124).
   */
  class cFailedParam
  {
  public:

    /**
     * \brief The three parts of a check.
     */
    enum ParamPart { QcClass,/** The first part of the check (QC1).*/
		     Group,  /** Second part of the check, giving the check group. */
		     Detail  /** Final part of the check, giving the exact check we're talking about.*/ 
    };
    
    /**
     * Constructor.
     *
     * \param param A string representation of the check (such as "QC1-2-24").
     */
    cFailedParam( const std::string &param );

    virtual ~cFailedParam( );
    
    /**
     * \brief Get a specific part of the check.
     *
     * \param part The part of the check to be returned.
     *
     * \returns The specified part of the
     * check. (<code>cFailedParam("QC1-2-23").getPart(cFailedParam::Detail)</code>
     * will return "23".)
     */
    virtual std::string getPart( ParamPart part ) const;

    /**
     * \brief Get the string representation of this check.
     *
     * \returns The string representation of this check.
     */
    virtual inline std::string toString( ) const
    { return cfailed; }

    /**
     */
    inline bool operator<( const cFailedParam &other ) const
    { return cfailed < other.toString(); }

    /**
     */
    inline bool operator==( const cFailedParam &other ) const
    { return cfailed == other.toString(); }

    /**
     * \brief An alternate way of comparing two cFailedParam objects,
     * ignoring everything after a colon.
     */
    struct altLess
    {
      bool operator()( const cFailedParam &main, const cFailedParam &other );
    };
    
  private:
    
    const std::string cfailed;
    
    struct Index
    {
      std::string::size_type start;
      std::string::size_type length;
    }
      indexes[3];
  };
  
  /**
   */
  typedef std::list<cFailedParam> cFailList;

  /**
   * \brief Get a list of \c cFailedParam objects.
   *
   * \param cfailed A cfailed string, as used in kvalobs
   * ("QC1-1-24:1,QC1-2-34:1").
   *
   * \returns A list of objects representing the different checks
   * given by \c cfailed.
   */
  cFailList getFailList( const std::string &cfailed );

  /**
   * \brief Get a \c kvData objects feiled checks as a list of
   * FailedParam objects.
   *
   * \param data the object from which to extract cfailed information.
   *
   * \returns A list of objects representing the different checks
   * given by \c data.
   */
  inline cFailList getFailList( const kvalobs::kvData &data )
  { return getFailList( data.cfailed() ); }
}

/**
 */
inline std::ostream &operator<<( std::ostream &stream, 
			  const QC::cFailedParam &cfailed )
{ return stream << cfailed.toString(); }

/**
 */
std::ostream &operator<<( std::ostream &stream, 
			  const QC::cFailList &failList );


#endif // __cFailedParam_h__
