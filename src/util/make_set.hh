/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#ifndef UTIL_MAKE_SET_HH
#define UTIL_MAKE_SET_HH 1

#include <set>

template<class SetType>
SetType make_set(const typename SetType::value_type& single)
{
  SetType s;
  s.insert(single);
  return s;
}

template<class SetType>
void insert_all(SetType& into, const SetType& from)
{
  into.insert(from.begin(), from.end());
}

template<class SetType>
class SetMaker
{
public:
  typedef typename SetType::value_type value_type;

  SetMaker& operator<<(const value_type& v)
    { mSet.insert(v); return *this; }

  SetType set() const
    { return mSet; }

private:
  SetType mSet;
};

#endif // UTIL_MAKE_SET_HH
