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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Colour.h"

#include <puTools/miStringFunctions.h>

#include <ostream>
#include <cstdio>

using namespace POptions;

std::map<std::string,Colour> Colour::cmap;

Colour::Colour(const values& va)
  : v(va)
{
}

Colour::Colour(uint32 h)
{
  uchar_t a= 255;
  if (h>0xFFFFFF)
    a= (h & 0xFF); h = (h >> 8);

  uchar_t b= (h & 0xFF); h = (h >> 8);
  uchar_t g= (h & 0xFF); h = (h >> 8);
  uchar_t r= (h & 0xFF);
  
  set(r,g,b,a);
}

Colour::Colour(const std::string& name_)
{
  int il;
  std::string lname = miutil::to_lower(name_);
  if ((il= lname.find(':'))>0){
    std::string s1= lname.substr(0,il);
    std::string s2= lname.substr(il+1,name_.size()-il-1);
    memberCopy(cmap[s1]);
    v.rgba[alpha]= atoi(s2.c_str());
  } else
    memberCopy(cmap[lname]);
}

Colour::Colour(const std::string& name_,
    uchar_t r, uchar_t g, uchar_t b, uchar_t a)
{
  std::string lname = miutil::to_lower(name_);
  define(lname,r,g,b,a);
  memberCopy(cmap[lname]);
}

Colour::Colour(const Colour &rhs)
{
  memberCopy(rhs);
}

Colour::~Colour()
{
}

Colour& Colour::operator=(const Colour &rhs)
{
  if (this != &rhs) 
    // elementwise copy
    memberCopy(rhs);

  return *this;
}

void Colour::memberCopy(const Colour& rhs)
{
  v= rhs.v;
  name= rhs.name;
  colourindex= rhs.colourindex;
}

void Colour::define(const std::string& name_,
    uchar_t r, uchar_t g, uchar_t b, uchar_t a)
{
  Colour c(r,g,b,a);
  std::string lname = miutil::to_lower(name_);
  c.name= lname;
  cmap[lname]= c;
}

void Colour::define(const std::string& name_, const values& va)
{
  Colour c(va);
  std::string lname = miutil::to_lower(name_);
  c.name= lname;
  cmap[lname]= c;
}

// hack: colourindex is platform-dependent
void Colour::setindex(const std::string& name_, const uchar_t index)
{
  std::string lname = miutil::to_lower(name_);
  cmap[lname].colourindex= index;
}

void Colour::definedColours(std::vector<Colour>& colourlist)
{
  colourlist.clear();
  std::map<std::string,Colour>::const_iterator it= cmap.begin();
  for (; it!=cmap.end(); it++)
    colourlist.push_back(it->second);
}

std::ostream& operator<<(std::ostream& out, const Colour& rhs)
{
  (void)rhs;
#if 0
  out << 
      " name: "  << rhs.name <<
      " red: "   << setw(3) << setfill('0') << int(rhs.v.rgba[0]) <<
      " green: " << setw(3) << setfill('0') << int(rhs.v.rgba[1]) <<
      " blue: "  << setw(3) << setfill('0') << int(rhs.v.rgba[2]) <<
      " alpha: " << setw(3) << setfill('0') << int(rhs.v.rgba[3]) <<
      " Index: " << int(rhs.colourindex);
#endif
  return out;
}
