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
#ifndef __QC__explainQC_h__
#define __QC__explainQC_h__

#include "cFailedParam.h"
#include <string>
#include <map>

namespace kvalobs
{
  class kvData;
}

namespace QC
{
  /**
   * \brief All functions giving a detailed explanation of of a check
   * must have this form.
   *
   * \param failParam The check which should be explained.
   * \param data The \c kvData object for which the check applies.
   *
   * \returns The explanation of \c failParam, possibly in context of
   * \c data.
   */
  typedef std::string(*Function)( const cFailedParam &failParam,
				  const kvalobs::kvData &data);

  /**
   * \brief Explains a single group of checks, such as "QC1-2-*".
   *
   * A group of checks has two types of explanations. First is the
   * explanation for the group itself. Second is the explanation of a
   * specific check within this group.
   */
  struct FailGroup
  {
    /**
     * \brief creates en empty (invalid) initialization..
     */
    FailGroup( );

    /**
     * \brief Constructs a FailGroup explanator, with group
     * explanation \c explanation, and detail explaining function \c
     * detailExpl.
     */ 
    FailGroup( const std::string &explanation,
	       Function detailExpl );

    /**
     * \brief The string explaining this particular group of checks.
     */
    const std::string explanation;

    /**
     * \brief A function providing detailed explanation of a check.
     */
    const Function getDetailExplanation;
  };

  /**
   * \brief A map of \c FailGroup objects for a specific QC class,
   * indexed by their check group number.
   */ 
  typedef std::map< std::string, FailGroup > FailGroupList;

  /**
   * \brief a map from QC classes to \c FailGroupList objects.
   */
  typedef std::map< std::string, FailGroupList > FailExplanation;

  /**
   * \brief The holder for all explanations of all possible QC checks.
   */
  extern FailExplanation failExpl;
}

#endif // __QC__explainQC_h__
