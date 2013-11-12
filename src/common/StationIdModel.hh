
#ifndef StationIdModel_hh
#define StationIdModel_hh 1

#include <QtCore/QAbstractTableModel>
#include <vector>

class StationIdModel : public QAbstractTableModel {
public:
  StationIdModel(QObject* parent=0);
  StationIdModel(const std::vector<int>& stationIds, QObject* parent=0);

  int rowCount(const QModelIndex&) const
    { return mStationIds.size(); }

  int columnCount(const QModelIndex&) const
    { return 2; }

  QVariant data(const QModelIndex& index, int role) const;

  int minStationId() const;
  int maxStationId() const;

private:
    std::vector<int> mStationIds;
};

#endif // StationIdModel_hh
