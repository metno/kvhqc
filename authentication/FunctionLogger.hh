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

#ifndef FUNCTIONLOGGER_HH_
#define FUNCTIONLOGGER_HH_

namespace hqc {
namespace debug {

class FunctionLogger {
public:
    FunctionLogger(const char * name)
        : name_(name) { log(true); }
    ~FunctionLogger()
        { log(false); }

private:
    void log(bool entering) const;

private:
    const char* name_;
    static int indent;
};

} // namespace debug
} // namespace hqc

#ifdef NDEBUG
#define LOG_FUNCTION() while(false) { }
#else // NDEBUG
#ifdef __GNUG__
#define LOG_FUNCTION() ::hqc::debug::FunctionLogger INTERNAL_function_logger(__PRETTY_FUNCTION__)
#else // __GNUG__
#define LOG_FUNCTION() ::hqc::debug::FunctionLogger INTERNAL_function_logger(__func__)
#endif // __GNUG__
#endif // NDEBUG

#endif /* FUNCTIONLOGGER_HH_ */
