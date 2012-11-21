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
#ifndef __enums_h__
#define  __enums_h__


namespace WatchRR
{
  enum Param {
    V4 = 34, V4S, V5, V5S, V6, V6S,
    RR_24 = 110,
    SD = 18, // Sn�dekke
    SA = 112 // Sn�dybde
  };

  enum ControlInfoFlags {
    fqclevel, fr, fcc, fs, fnum, fpos, fmis, ftime,
    fw, fstat, fcp, fclim, fd, fpre, fcombi, fhqc 
  };
}

#endif //  __enums_h__
