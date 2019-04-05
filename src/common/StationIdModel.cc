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

#include "StationIdModel.hh"

#include "KvMetaDataBuffer.hh"

#define MILOGGER_CATEGORY "kvhqc.StationIdModel"
#include "util/HqcLogging.hh"

namespace /* anonymous */ {
struct int_by_text : public std::binary_function<int, int, bool> {
  typedef std::pair<float, int> key_t;
  key_t key(int i) const { if (i<=0) return std::make_pair(0, 0.0f); int l = (int)std::log10(i); float f = i / std::pow(10, l); return std::make_pair(f, l); }
  bool operator() (int a, int b) const { return key(a) < key(b); }
};
} // namespace anonymous

StationIdModel::StationIdModel(QObject* parent)
  : QAbstractTableModel(parent)
{
  METLIBS_LOG_SCOPE();
  if (not KvMetaDataBuffer::instance())
    return;

  const hqc::kvStation_v& stations = KvMetaDataBuffer::instance()->allStations();
  for (hqc::kvStation_v::const_iterator it = stations.begin(); it != stations.end(); ++it) {
    mStationIds.push_back(it->stationID());
  }
  std::sort(mStationIds.begin(), mStationIds.end(), int_by_text());
}

StationIdModel::StationIdModel(const std::vector<int>& stationIds, QObject* parent)
  : QAbstractTableModel(parent)
  , mStationIds(stationIds)
{
  METLIBS_LOG_SCOPE();
  std::sort(mStationIds.begin(), mStationIds.end(), int_by_text());
}

QVariant StationIdModel::data(const QModelIndex& index, int role) const
{
  //METLIBS_LOG_SCOPE();
  if (role == Qt::DisplayRole or role == Qt::EditRole) {
    const int stationId = mStationIds[index.row()];
    if (index.column() == 0)
      return QString::number(stationId);
    try {
      const kvalobs::kvStation& s = KvMetaDataBuffer::instance()->findStation(stationId);
      return QString::fromStdString(s.name());
    } catch (std::exception& e) {
      return "?";
    }
  }
  return QVariant();
}

int StationIdModel::minStationId() const
{
  if (mStationIds.empty())
    return 60;

  return mStationIds.front();
}

int StationIdModel::maxStationId() const
{
  if (mStationIds.empty())
    return 100000;

  return mStationIds.back();
}
