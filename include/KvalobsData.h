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

#ifndef KVALOBSDATA_H_
#define KVALOBSDATA_H_

#include <QString>
#include <puTools/miTime>
#include <boost/shared_ptr.hpp>
#include <string>

namespace kvalobs
{
  class kvControlInfo;
  class kvUseInfo;
}

namespace model
{

/**
 * Internal storage for kvalobs data
 *
 * @note Internally, this class contains a pointer to stored data. This means
 * that modifying an instance of this class also will modify any copies that
 * have been made earlier.
 */
class KvalobsData
{
public:
  KvalobsData();
  ~KvalobsData();

  std::size_t size() const;
  bool empty() const;

  const int stnr() const;
  void set_stnr(int value);

  const int snr() const;
  void set_snr(int value);

  const QString & name() const;
  void set_name(const QString & value);

  const miutil::miTime & otime() const;
  void set_otime(const miutil::miTime & value);

  const miutil::miTime & tbtime() const;
  void set_tbtime(const miutil::miTime & value);

  const int showTypeId() const;
  void set_showTypeId(int value);

  const int typeIdChanged() const;
  void set_typeIdChanged(int value);

  const int typeId(std::size_t parameter) const;
  void set_typeId(std::size_t parameter, int value);

  const double orig(std::size_t parameter) const;
  void set_orig(std::size_t parameter, double value);

  const int flag(std::size_t parameter) const;
  void set_flag(std::size_t parameter, int value);

  const double corr(std::size_t parameter) const;
  void set_corr(std::size_t parameter, double value);

  const int level(std::size_t parameter) const;
  void set_level(std::size_t parameter, int value);

  const int sensor(std::size_t parameter) const;
  void set_sensor(std::size_t parameter, int value);

  const kvalobs::kvControlInfo & controlinfo(std::size_t parameter) const;
  void set_controlinfo(std::size_t parameter, const kvalobs::kvControlInfo & value);

  const kvalobs::kvUseInfo & useinfo(std::size_t parameter) const;
  void set_useinfo(std::size_t parameter, const kvalobs::kvUseInfo & value);

  const std::string cfailed(std::size_t parameter) const;
  void set_cfailed(std::size_t parameter, const std::string & value);

private:
  class Impl;

  Impl & impl();
  const Impl & impl() const;

  boost::shared_ptr<Impl> impl_;
};

typedef std::vector<KvalobsData> KvalobsDataList;
typedef boost::shared_ptr<KvalobsDataList> KvalobsDataListPtr;


}

#endif /* KVALOBSDATA_H_ */
