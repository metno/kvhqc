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

namespace hqc {
namespace debug {

class ScopeLogger {
public:
    ScopeLogger(const char * name, int line)
        : name_(name) { log(line); }
    ~ScopeLogger()
        { log(-1); }

private:
    /// \param line line number for entering, or -1 for leaving
    void log(int line) const;

private:
    const char* name_;
    static int indent;
};

} // namespace debug
} // namespace hqc

#ifdef NDEBUG
#define LOG_SCOPE()    do { /* nothing */ } while(false)
#define DBG(x)         do { /* nothing */ } while(false)
#define DBGL           do { /* nothing */ } while(false)
#define DBG1(x) 
#define DBGV(x)        do { /* nothing */ } while(false)
#define DBGE(x)
#else // NDEBUG
#include <iostream>
#ifdef __GNUG__
#define LOG_SCOPE() ::hqc::debug::ScopeLogger INTERNAL_scope_logger(__PRETTY_FUNCTION__, __LINE__)
#else // __GNUG__
#define LOG_SCOPE() ::hqc::debug::ScopeLogger INTERNAL_scope_logger(__func__, __LINE__)
#endif // __GNUG__
#define DBGHDR std::cout << __FILE__ << ":" << __LINE__ << "[" << __FUNCTION__ << "]"
#define DBG(x) do { DBGHDR << "\n    " << x << std::endl; } while(false)
#define DBGL   do { DBGHDR << std::endl; } while(false)
#define DBG1(x) " " #x "='" << x << "'"
#define DBGV(x) DBG(DBG1(x))
#define DBGE(x) x
#endif // NDEBUG

#endif /* DEBUG_HH_ */
