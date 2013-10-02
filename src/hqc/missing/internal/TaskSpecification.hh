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

#ifndef TASKSPECIFICATION_H_
#define TASKSPECIFICATION_H_

//#include "internal/TypeIDMap.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <list>
#include <stdexcept>



/**
 * Specification of what data set we want to search for missing observations.
 *
 * The specification consists of a time range, and optionally a typeid.
 *
 * \ingroup group_data_analysis
 */
class TaskSpecification
{
    boost::gregorian::date from_;
    boost::gregorian::date to_;
	int typeID_;
	const char * type_;
public:
	static const int paramID_ = 110;

	/**
	 * Construct, with the given time range, and optionally typeid. If typeid
	 * is 0 or ommitted, any typeids are requested.
	 *
	 * \param from earliest time we want to analyze
	 * \param to latest time we want to analyze
	 * \param type kvalobs typeid we want to search. If this value is zero or
	 * not given, we will analyze all typeids.
	 */
    TaskSpecification(boost::gregorian::date from, boost::gregorian::date to, int type = 0) :
		from_(from), to_(to), typeID_(type)
	{
//		for (internal::ITypeIDMap it = internal::typeID.begin(); it
//				!= internal::typeID.end(); ++it)
//		{
//			if (it->second == typeID_)
//			{
//				type_ = it->first;
//				break;
//			}
//		}
	}


	/**
	 * Get the earliest date we want to analyze
	 */
    inline const boost::gregorian::date & from() const
	{
		return from_;
	}
	/**
	 * Get the latest date we want to analyze
	 */
    inline const boost::gregorian::date & to() const
	{
		return to_;
	}

	/**
	 * Get the typeid we want to analyze
	 */
	inline int typeID() const
	{
		return typeID_;
	}

	/**
	 * Get the type name of the data we want to analyze
	 */
	inline const char * type() const
	{
		return type_;
	}

	/**
	 * Get the parameter id, which we use to check for an observation in the
	 * database. This is RR_24, which all stations we may analyze must report
	 * in every message.
	 */
	inline int paramID() const
	{
		return paramID_;
	}

	/**
	 * Is the given piece of data relevant to this specification, based on
	 * typeid.
	 *
	 * \tparam Data Any object which have a typeID function returning an int, such as kvalobs::kvData.
	 * \param data the piece of data to analyze
	 */
	template<class Data>
	bool isRelevant(const Data & data) const
	{
		return (typeID() == 0 and data.typeID() > 0) or typeID()
				== data.typeID();
	}

};

#endif /*TASKSPECIFICATION_H_*/
