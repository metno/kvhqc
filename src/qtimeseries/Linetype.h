/*
  libqTimeseries - Qt classes for time series plots

  Copyright (C) 2006-2014 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef QTIMESERIES_LINETYPE_H
#define QTIMESERIES_LINETYPE_H

#include <puCtools/porttypes.h>

#include <map>
#include <string>
#include <vector>

namespace POptions {

class Linetype {

public:
  Linetype();
  Linetype(const std::string& _name);
  ~Linetype();
  Linetype& operator=(const Linetype &rhs);
  bool operator==(const Linetype &rhs) const;

  static void init();

  static void define(const std::string& _name,
      uint16 _bmap= 0xFFFF, int _factor= 1);

  std::string name;
  bool     stipple;
  uint16   bmap;
  int      factor;

private:
  static std::map<std::string,Linetype> linetypes;
  static std::vector<std::string> linetypeSequence;
  static Linetype defaultLinetype;

  void memberCopy(const Linetype& rhs);
};

} // namespace POptions

#endif // QTIMESERIES_LINETYPE_H
