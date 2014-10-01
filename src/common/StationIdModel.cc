
#include "StationIdModel.hh"

#include "KvMetaDataBuffer.hh"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#define MILOGGER_CATEGORY "kvhqc.StationIdModel"
#include "util/HqcLogging.hh"

namespace /* anonymous */ {
struct int_by_text : public std::binary_function<int, int, bool> {
  bool operator() (int a, int b) const;
};
bool int_by_text::operator() (int a, int b) const
{
  const std::string ta = boost::lexical_cast<std::string>(a), tb = boost::lexical_cast<std::string>(b);
  return ta < tb;
}
} // namespace anonymous

StationIdModel::StationIdModel(QObject* parent)
  : QAbstractTableModel(parent)
{
  METLIBS_LOG_SCOPE();
  const std::list<kvalobs::kvStation>& stations = KvMetaDataBuffer::instance()->allStations();
  BOOST_FOREACH(const kvalobs::kvStation& s, stations) {
    mStationIds.push_back(s.stationID());
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
