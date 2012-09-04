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

#include "FunctionLogger.hh"

#include <Qt/qdebug.h>
#include <sstream>
#include <sys/time.h>

namespace hqc {
namespace debug {

void FunctionLogger::log(bool entering) const
{
    if( entering )
        indent += 1;
    std::ostringstream data;
    for ( int i = 0; i < indent; ++ i )
      data << "--+--";
    data << '>' << ( entering ? " Entering " : " Leaving ") << name_;

    struct timeval now;
    gettimeofday(&now, 0);
    struct tm *tmp = localtime(&now.tv_sec);
    if (tmp != NULL) {
        char txt[64], mtxt[12];
        strftime(txt, sizeof(txt), "%F %T", tmp);
        snprintf(mtxt, sizeof(mtxt), "%06ld", now.tv_usec);
        data << " [" << txt << '.' << mtxt << ']';
    }

    qDebug() << data.str().c_str();
    if( !entering )
        indent -= 1;
}

int FunctionLogger::indent = 3;

} // namespace debug
} // namespace hqc
