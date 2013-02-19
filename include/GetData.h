/* -*- c++ -*-

  Kvalobs - Free Quality Control Software for Meteorological Observations

  Copyright (C) 2013 met.no

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
  with KVALOBS; if not, write to the Free Software Foundation Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __GETDATA_H__
#define __GETDATA_H__

#include <kvcpp/KvGetDataReceiver.h>

class HqcMainWindow;

class GetData : public kvservice::KvGetDataReceiver
{
public:
    GetData(HqcMainWindow* o);
    HqcMainWindow* w;
    /**
     * next, this function is called for every data set!
     *
     * \datalist the data.
     * \return true if we shall continue. False if you want to
     *         stop retriving data from kvalobs.
     */
    bool next(kvservice::KvObsDataList &datalist);
};

#endif
