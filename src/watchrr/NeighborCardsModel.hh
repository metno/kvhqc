
#ifndef WATCHRR_NEIGHBORCARDSMODEL_HH
#define WATCHRR_NEIGHBORCARDSMODEL_HH 1

#include "common/DataItem.hh"
#include <vector>

#include <QtCore/QAbstractTableModel>

class NeighborCardsModel : public QAbstractTableModel
{   Q_OBJECT;
public:
  NeighborCardsModel(EditAccessPtr da/*, ModelAccessPtr ma*/, const Sensor& sensor, const TimeRange& timeRange);
  virtual ~NeighborCardsModel();

  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  void setTime(const timeutil::ptime& time);
  const timeutil::ptime& getTime() const
    { return mTime; }

  std::vector<int> neighborStations() const;

Q_SIGNALS:
  void timeChanged(const timeutil::ptime& time);

private:
  EditDataPtr getObs(const QModelIndex& index) const;
  SensorTime getSensorTime(const QModelIndex& index) const;
  DataItemPtr getItem(const QModelIndex& index) const
    { return mItems[index.column()]; }
  timeutil::ptime getTime(const QModelIndex& index) const
    { return mTime + mTimeOffsets[index.column()]; }
  void onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);

private:
  EditAccessPtr mDA;
  TimeRange mTimeRange;
  timeutil::ptime mTime;

  // rows
  std::vector<DataItemPtr> mItems;
  std::vector<boost::posix_time::time_duration> mTimeOffsets;

  // columns
  std::vector<Sensor> mSensors;
};

#endif /* WATCHRR_NEIGHBORCARDSMODEL_HH */
