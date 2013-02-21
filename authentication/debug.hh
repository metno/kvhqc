/* -*- c++ -*-
  Kvalobs - Free Quality Control Software for Meteorological Observations

  Copyright (C) 2011 met.no

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

#ifndef DEBUG_HH_
#define DEBUG_HH_

#include "HqcLogging.hh"
#include <string>

namespace hqc {
namespace debug {

class ScopeLogger {
public:
    ScopeLogger(const char* category, const char* function);
    ~ScopeLogger();

private:
    void log(const char* txt);

    log4cpp::Category& category;
    const char* function;
    static int indent;
};

} // namespace debug
} // namespace hqc

#ifdef NDEBUG
#define LOG_SCOPE(c)   do { /* nothing */ } while(false)
#define DBG(x)         do { /* nothing */ } while(false)
#define DBGL           do { /* nothing */ } while(false)
#define DBG1(x) 
#define DBGV(x)        do { /* nothing */ } while(false)
#define DBGE(x)
#else // NDEBUG
#include "HqcLogging.hh"
#define LOG_SCOPE(category) const ::hqc::debug::ScopeLogger INTERNAL_scope_logger(category, __PRETTY_FUNCTION__)
#define DBG(x) LOG4HQC_DEBUG("hqc", x)
#define DBGL   LOG4HQC_DEBUG("hqc", "L:" << __LINE__)
#define DBG1(x) " " #x "='" << x << "'"
#define DBGV(x) DBG(DBG1(x))
#define DBGE(x) x
#endif // NDEBUG

#endif /* DEBUG_HH_ */
