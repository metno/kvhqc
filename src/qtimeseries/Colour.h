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

#ifndef QTIMESERIES_DICOLOUR_H
#define QTIMESERIES_DICOLOUR_H

#include <puCtools/porttypes.h>

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

namespace POptions {

class Colour {
public:
  enum cIndex{
    red   =0,
    green =1,
    blue  =2,
    alpha =3
  };
  enum {maxv= 255};
  struct values{
    uchar_t rgba[4];
    inline values();
    inline values& operator=(const values &rhs);
    inline bool operator==(const values &rhs) const ;
  };
private:
  std::string name;
  values v;
  uchar_t colourindex;
  static std::map<std::string,Colour> cmap;

  void memberCopy(const Colour& rhs);

public:
  Colour(const std::string& name_);
  Colour(const values& va);
  Colour(uint32 hexv=0);
  Colour(uchar_t r, uchar_t g, uchar_t b, uchar_t a=maxv);
  Colour(const std::string& name_,
      uchar_t r, uchar_t g, uchar_t b, uchar_t a=maxv);
  Colour(const Colour &rhs);
  ~Colour();

  Colour& operator=(const Colour &rhs);
  bool operator==(const Colour &rhs) const;

  // static functions for static colour-map
  static void define(const std::string&, uchar_t r, uchar_t g,
      uchar_t b, uchar_t a=maxv);
  static void define(const std::string&, const values&);
  static void setindex(const std::string&, uchar_t);
  static void definedColours(std::vector<Colour>& colourlist);

  void set(uchar_t r, uchar_t g, uchar_t b, uchar_t a =maxv)
    { v.rgba[red]=r; v.rgba[green]=g; v.rgba[blue]=b; v.rgba[alpha]=a; }

  void set(const values& va)
    {v= va;}

  void set(const cIndex i,const uchar_t b){v.rgba[i]=b;}

  uchar_t R() const {return v.rgba[red];   }
  uchar_t G() const {return v.rgba[green]; }
  uchar_t B() const {return v.rgba[blue];  }
  uchar_t A() const {return v.rgba[alpha]; }

  float fR() const {return 1.0*v.rgba[red]/maxv;  }
  float fG() const {return 1.0*v.rgba[green]/maxv;}
  float fB() const {return 1.0*v.rgba[blue]/maxv; }
  float fA() const {return 1.0*v.rgba[alpha]/maxv;}

  const uchar_t* RGBA() const {return v.rgba; }
  const uchar_t* RGB()  const {return v.rgba; }
  uchar_t Index() const {return colourindex; }

  const std::string& Name() const {return name;}

  friend std::ostream& operator<<(std::ostream& out, const Colour& rhs);
};

// inline Colour::values member functions

inline Colour::values::values(){
  rgba[0]=0; rgba[1]=0;
  rgba[2]=0; rgba[3]=0;
}

inline Colour::values& Colour::values::operator=(const Colour::values &rhs){
  if (this != &rhs){
    rgba[0]= rhs.rgba[0];
    rgba[1]= rhs.rgba[1];
    rgba[2]= rhs.rgba[2];
    rgba[3]= rhs.rgba[3];
  }
  return *this;
}

inline bool Colour::values::operator==(const Colour::values &rhs) const {
  return (rgba[0]==rhs.rgba[0] &&
      rgba[1]==rhs.rgba[1] &&
      rgba[2]==rhs.rgba[2] &&
      rgba[3]==rhs.rgba[3]);
}

} // namespace POptions

#endif // QTIMESERIES_DICOLOUR_H
