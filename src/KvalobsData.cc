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
#include <QString>
#include <algorithm>
#include <functional>
#include <map>

namespace model
{

  namespace {
    /// Internal missing representation
    const int missing_ = -32767;
  }

class KvalobsData::Impl
{
public:
  int stnr;
  int snr;
  QString name;
  double latitude;
  double longitude;
  double altitude;
  miutil::miTime otime;
  miutil::miTime tbtime;
  int showTypeId;
  int typeIdChanged;

  Impl()
  : stnr(0), snr(0), showTypeId(0), typeIdChanged(0), latitude(0), longitude(0), altitude(0)
  {}

  /**
   * Parameter-specific data
   */
  struct ObservationData
  {
    ObservationData() :
      typeId(missing_),
      original(missing_),
      corrected(missing_),
      level(0),
      sensor(0)
    {}

    int typeId;
    double original;
    double corrected;
    int level;
    int sensor;
    kvalobs::kvControlInfo controlinfo;
    kvalobs::kvUseInfo useinfo;
    std::string cfailed;
  };

  typedef std::map<int, ObservationData> ParameterSortedObservationData;
  ParameterSortedObservationData parameters;

  std::size_t size() const
  {
    return parameters.size();
  }

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
  impl_ = boost::shared_ptr<Impl>(new Impl);
}

KvalobsData::~KvalobsData()
{
//  delete impl_;
}

std::size_t KvalobsData::size() const
{
  return impl().size();
}

bool KvalobsData::empty() const
{
  return impl().size() == 0;
}

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

double KvalobsData::latitude() const
{
  return impl().latitude;
}
void KvalobsData::set_latitude(double value)
{
  impl().latitude = value;
}

double KvalobsData::longitude() const
{
 return impl().longitude;
}
void KvalobsData::set_longitude(double value)
{
  impl().longitude = value;
}

double KvalobsData::altitude() const
{
  return impl().altitude;
}
void KvalobsData::set_altitude(double value)
{
  impl().altitude = value;
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

namespace
{
  /*
  Convert to "Diana-value" of range check flag
  */
  int numCode1(int nib) {
    int code = 9;
    if ( nib == 0 )
      code = 0;
    else if ( nib == 1 )
      code = 1;
    else if ( nib == 2 || nib == 3 )
      code = 2;
    else if ( nib == 4 || nib == 5 )
      code = 3;
    else if ( nib == 6 )
      code = 9;
    return code;
  }

  /*
  Convert to "Diana-value" of consistency check flag
  */
  int numCode2(int nib) {
    int code = 9;
    if ( nib == 0 )
      code = 0;
    else if ( nib == 1 )
      code = 1;
    else if ( nib >= 2 && nib <= 7 )
      code = 8;
    else if ( nib == 10 || nib == 11 )
      code = 7;
    else if ( nib == 13 )
      code = 9;
    return code;
  }

  /*
  Convert to "Diana-value" of prognostic space control flag
  */
  int numCode3(int nib) {
    int code = 9;
    if ( nib == 0 )
      code = 0;
    else if ( nib == 1 )
      code = 1;
    else if ( nib == 2 || nib == 3 )
      code = 2;
    else if ( nib == 4 || nib == 5 )
      code = 3;
    else if ( nib == 6 )
      code = 5;
    return code;
  }

  /*
  Convert to "Diana-value" of step check flag
  */
  int numCode4(int nib) {
    int code = 9;
    if ( nib == 0 )
      code = 0;
    else if ( nib == 1 )
      code = 1;
    else if ( nib == 2 || nib == 3 )
      code = 2;
    else if ( nib >= 4 && nib <= 7 )
      code = 8;
    else if ( nib == 9 || nib == 10 )
      code = 7;
    return code;
  }

  /*
  Convert to "Diana-value" of timeseries adaption flag
  */
  int numCode5(int nib) {
    int code = 9;
    if ( nib == 0 )
      code = 0;
    else if ( nib == 1 || nib == 2 )
      code = 5;
    else if ( nib == 3 )
      code = 3;
    return code;
  }

  /*
  Convert to "Diana-value" of statistics control flag
  */
  int numCode6(int nib) {
    int code = 9;
    if ( nib == 0 )
      code = 0;
    else if ( nib == 1 )
      code = 1;
    else if ( nib == 2 )
      code = 3;
    return code;
  }

  /*
  Convert to "Diana-value" of climatology control flag
  */
  int numCode7(int nib) {
    int code = 9;
    if ( nib == 0 )
      code = 0;
    else if ( nib == 1 )
      code = 1;
    else if ( nib == 2 )
      code = 3;
    else if ( nib == 3 )
      code = 7;
    return code;
  }

  /*
  Convert to "Diana-value" of HQC flag
  */
  int numCode8(int nib) {
    int code = 9;
    if ( nib <  10 )
      code = nib;
    else
      code = 9;
    return code;
  }

}

const int KvalobsData::flag(std::size_t parameter) const
{
  const kvalobs::kvControlInfo & controlInfo = controlinfo(parameter);

  // Find flags from the different checks

  int nib1  =controlInfo.flag(1);
  int nib2  =controlInfo.flag(2);
  int nib3  =controlInfo.flag(4);
  int nib4  =controlInfo.flag(3);
  int nib5  =controlInfo.flag(7);
  int nib6  =controlInfo.flag(9);
  int nib7  =controlInfo.flag(11);
  int nib8  =controlInfo.flag(10);
  int nib9  =controlInfo.flag(12);
  int nib10 =controlInfo.flag(15);
  // Decode flags

  int nc1 = numCode1(nib1); // Range check
  int nc2 = numCode2(nib2); // Formal Consistency check
  int nc8 = numCode2(nib8); // Climatologic Consistency check
  // Use the largest value from these checks
  nc1 = nc1 > nc2 ? nc1 : nc2;
  nc1 = nc1 > nc8 ? nc1 : nc8;
  nc2 = numCode3(nib3); //Prognostic space control
  int nc3 = numCode4(nib4); //Step check
  int nc4 = numCode5(nib5); //Timeseries adaption
  int nc5 = numCode6(nib6); //Statistics control
  int nc6 = numCode7(nib7); //Climatology control
  // Use the largest value from the three last checks
  nc4 = nc4 > nc5 ? nc4 : nc5;
  nc4 = nc4 > nc6 ? nc4 : nc6;
  if ( nib9 > 1 )
    nc4 = nc4 > 6 ? nc4 : 6;
  nc5 = numCode8(nib10);

  int nc = 10000*nc1 + 1000*nc2 + 100*nc3 + 10*nc4 + nc5;
  return nc;
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

kvalobs::kvData KvalobsData::getKvData(std::size_t paramid) const
{
  return kvalobs::kvData(
      stnr(),
      otime(),
      orig(paramid),
      paramid,
      tbtime(),
      typeId(paramid),
      sensor(paramid),
      level(paramid),
      corr(paramid),
      controlinfo(paramid),
      useinfo(paramid),
      cfailed(paramid)
      );
}

KvalobsData::Impl & KvalobsData::impl()
{
  return * impl_;
//  return * static_cast<KvalobsData::Impl *>(impl_.data());
}

const KvalobsData::Impl & KvalobsData::impl() const
{
  return * impl_;
//  return * static_cast<const KvalobsData::Impl *>(impl_.data());
}


}
