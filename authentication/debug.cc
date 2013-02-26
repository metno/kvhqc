/* -*- c++ -*-
  Kvalobs - Free Quality Control Software for Meteorological Observations

  Copyright (C) 2013 met.no

  Contact information:
  Norwegian Meteorological Institute
  Postboks 43 Blindern
  N-0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of KVALOBS

  KVALOBS is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  KVALOBS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with KVALOBS; if not, write to the Free Software Foundation Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "debug.hh"

#include <sstream>

namespace hqc {
namespace debug {

ScopeLogger::ScopeLogger(const char* cat, const char* fun)
    : mCategory(log4cpp::Category::getInstance(cat))
    , mFunction(fun)
{
    sIndent += 1;
    log("> ENTER ");
}
 
ScopeLogger::~ScopeLogger()
{
    log("< LEAVE ");
    sIndent -= 1;
}

void ScopeLogger::log(const char* txt)
{
    if (mCategory.isPriorityEnabled(log4cpp::Priority::DEBUG)) {
        std::ostringstream indent;
        for (int i = 0; i < sIndent; ++ i)
            indent << "+-";
        mCategory << log4cpp::Priority::DEBUG
                  << indent.str() << txt << mFunction;
    }
}

int ScopeLogger::sIndent = 2;

} // namespace debug
} // namespace hqc
