/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 Copyright (C) 2010 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
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

#include "../include/KvalobsData.h"
#include <kvalobs/flag/kvControlInfo.h>
#include <kvalobs/flag/kvUseInfo.h>
#include <QSharedData>
#include <algorithm>
#include <functional>
#include <map>

namespace model
{

  namespace {
    const int NOPARAM = 1043;
    const int missing_ = -32767;
  }

class KvalobsData::Impl : public QSharedData
{
public:
  int stnr;
  int snr;
  QString name;
  miutil::miTime otime;
  miutil::miTime tbtime;
  int showTypeId;
  int typeIdChanged;

  Impl()
  : stnr(0), snr(0), showTypeId(0), typeIdChanged(0)
  {}

  struct ObservationData
  {
    ObservationData() :
      typeId(missing_),
      original(missing_),
      flag(0),
      corrected(missing_),
      level(0),
      sensor(0)
    {}

    int typeId;
    double original;
    int flag;
    double corrected;
    int level;
    int sensor;
    kvalobs::kvControlInfo controlinfo;
    kvalobs::kvUseInfo useinfo;
    std::string cfailed;
  };

  typedef std::map<int, ObservationData> ParameterSortedObservationData;
  ParameterSortedObservationData parameters;

  const ObservationData * obsData(int parameter) const {
    ParameterSortedObservationData::const_iterator find = parameters.find(parameter);
    if ( find == parameters.end() )
      return 0;
    return & find->second;

  }
  ObservationData * obsData(int parameter) {
    return & parameters[parameter];
  }
};



KvalobsData::KvalobsData()
{
  impl_ = new Impl;
}

//KvalobsData::KvalobsData(const KvalobsData & toCopy)
//{
//  impl_ = new Impl(* toCopy.impl_);
//}


KvalobsData::~KvalobsData()
{
//  delete impl_;
}

//const KvalobsData & KvalobsData::operator = (const KvalobsData & toCopy)
//{
//  delete impl_;
//  impl_ = new Impl(* toCopy.impl_);
//}

const int KvalobsData::stnr() const
{
  return impl().stnr;
}
void KvalobsData::set_stnr(int value)
{
  impl().stnr = value;
}

const int KvalobsData::snr() const
{
  return impl().snr;
}
void KvalobsData::set_snr(int value)
{
  impl().snr = value;
}

const QString & KvalobsData::name() const
{
  return impl().name;
}
void KvalobsData::set_name(const QString & value)
{
  impl().name = value;
}

const miutil::miTime & KvalobsData::otime() const
{
  return impl().otime;
}
void KvalobsData::set_otime(const miutil::miTime & value)
{
  impl().otime = value;
}


const miutil::miTime & KvalobsData::tbtime() const
{
  return impl().tbtime;
}
void KvalobsData::set_tbtime(const miutil::miTime & value)
{
  impl().tbtime = value;
}


const int KvalobsData::showTypeId() const
{
  return impl().showTypeId;
}
void KvalobsData::set_showTypeId(int value)
{
  impl().showTypeId = value;
}


const int KvalobsData::typeIdChanged() const
{
  return impl().typeIdChanged;
}
void KvalobsData::set_typeIdChanged(int value)
{
  impl().typeIdChanged = value;
}


const int KvalobsData::typeId(std::size_t parameter) const
{
  const Impl::ObservationData * obs = impl().obsData(parameter);
  if ( ! obs )
    return missing_;
  return obs->typeId;
}
void KvalobsData::set_typeId(std::size_t parameter, int value)
{
  impl().obsData(parameter)->typeId = value;
}

const double KvalobsData::orig(std::size_t parameter) const
{
  const Impl::ObservationData * obs = impl().obsData(parameter);
  if ( ! obs )
    return missing_;
  return obs->original;
}
void KvalobsData::set_orig(std::size_t parameter, double value)
{
  impl().obsData(parameter)->original = value;
}


const int KvalobsData::flag(std::size_t parameter) const
{
  const Impl::ObservationData * obs = impl().obsData(parameter);
  if ( ! obs )
    return 0;
  return obs->flag;
}
void KvalobsData::set_flag(std::size_t parameter, int value)
{
  impl().obsData(parameter)->flag = value;
}


const double KvalobsData::corr(std::size_t parameter) const
{
  const Impl::ObservationData * obs = impl().obsData(parameter);
  if ( ! obs )
    return missing_;
  return obs->corrected;
}
void KvalobsData::set_corr(std::size_t parameter, double value)
{
  impl().obsData(parameter)->corrected = value;
}

const int KvalobsData::level(std::size_t parameter) const
{
  const Impl::ObservationData * obs = impl().obsData(parameter);
  if ( ! obs )
    return 0;
  return obs->level;
}
void KvalobsData::set_level(std::size_t parameter, int value)
{
  impl().obsData(parameter)->level = value;
}


const int KvalobsData::sensor(std::size_t parameter) const
{
  const Impl::ObservationData * obs = impl().obsData(parameter);
  if ( ! obs )
    return 0;
  return obs->sensor;
}
void KvalobsData::set_sensor(std::size_t parameter, int value)
{
  impl().obsData(parameter)->sensor = value;
}


const kvalobs::kvControlInfo & KvalobsData::controlinfo(std::size_t parameter) const
{
  const Impl::ObservationData * obs = impl().obsData(parameter);
  if ( ! obs ) {
    static const kvalobs::kvControlInfo defaultControlInfo("0000003000000000");
    return defaultControlInfo;
  }
  return obs->controlinfo;
}
void KvalobsData::set_controlinfo(std::size_t parameter, const kvalobs::kvControlInfo & value)
{
  impl().obsData(parameter)->controlinfo = value;
}


const kvalobs::kvUseInfo & KvalobsData::useinfo(std::size_t parameter) const
{
  const Impl::ObservationData * obs = impl().obsData(parameter);
  if ( ! obs ) {
      static const kvalobs::kvUseInfo defaultUseInfo;
      return defaultUseInfo;
  }
  return obs->useinfo;
}
void KvalobsData::set_useinfo(std::size_t parameter, const kvalobs::kvUseInfo & value)
{
  impl().obsData(parameter)->useinfo = value;
}


const std::string KvalobsData::cfailed(std::size_t parameter) const
{
  const Impl::ObservationData * obs = impl().obsData(parameter);
  if ( ! obs )
    return std::string();
  return obs->cfailed;
}
void KvalobsData::set_cfailed(std::size_t parameter, const std::string & value)
{
  impl().obsData(parameter)->cfailed = value;
}

KvalobsData::Impl & KvalobsData::impl()
{
  return * static_cast<KvalobsData::Impl *>(impl_.data());
}

const KvalobsData::Impl & KvalobsData::impl() const
{
  return * static_cast<const KvalobsData::Impl *>(impl_.data());
}


}
