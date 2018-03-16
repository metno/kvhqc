/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2014-2018 met.no

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

#ifndef HQC_SYSTEM_DB
#define HQC_SYSTEM_DB 1

#include "common/KvTypedefs.hh"

#include <QString>
#include <QStringList>

#include <map>

class HqcSystemDB {
public:
  static QString explainCheck(QString& check);
  static bool paramLimits(int paramid, float& low, float& high);
  static bool shownDecimals(int paramid, int& decimals);

  struct ParamCode {
    int value;
    QString longText;
    QStringList shortTexts;
  };
  typedef QList<ParamCode> ParamCode_ql;

  static ParamCode_ql paramCodes(int paramid);

  typedef std::map<int, int> station2prio_t;
  static station2prio_t stationPriorities();

  static hqc::int_s coastalStations();

  static QString remappedCounty(int countryid, int municip_code);

  static QString explainFlagValue(int fn, int fv);

  static hqc::int_v relatedParameters(int paramid, const QString& viewType);

  static hqc::int_s ignoredParameters(const QString& viewType);

  static void aggregatedParameters(int paramFrom, hqc::int_s& paramTo);

  struct Region {
    QString regionLabel;
    QStringList countyLabels;
    QStringList countyDbNames;
  };
  typedef QList<Region> Region_ql;

  static Region_ql regions();

  struct ParamGroup {
    QString label;
    hqc::int_v paramIds;
  };
  typedef QList<ParamGroup> ParamGroup_ql;

  static ParamGroup_ql paramGroups();
};

#endif // HQC_SYSTEM_DB
