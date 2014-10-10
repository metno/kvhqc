
#ifndef DATACOLUMN_HH
#define DATACOLUMN_HH 1

#include "DataItem.hh"
#include "ObsColumn.hh"
#include "common/EditAccess.hh"
#include "common/TimeBuffer.hh"

class DataColumn : public ObsColumn
{ Q_OBJECT;

public:
  DataColumn(EditAccess_p ea, const Sensor& sensor, const TimeSpan& time, DataItem_p item);
  ~DataColumn();

  void setHeaderShowStation(bool show)
    { mHeaderShowStation = show; }

  virtual void attach(ObsTableModel* table);

  virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
  virtual QVariant data(const timeutil::ptime& time, int role) const;
  virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);
  virtual QVariant headerData(Qt::Orientation orientation, int role) const;

  DataItem_p getItem() const
    { return mItem; }
  virtual bool matchSensor(const Sensor& sensorObs) const;

  virtual const boost::posix_time::time_duration& timeOffset() const
    { return mTimeOffset; }
  void setTimeOffset(const boost::posix_time::time_duration& timeOffset);

  void setTimeSpan(const TimeSpan& tr);

  virtual const Sensor& sensor() const;
  virtual Time_s times() const;
  virtual int type() const
    { return mItem->type(); }

  virtual bool isBusy() const
    { return mRequestBusy; }

  ObsData_p getObs(const Time& time) const;
  SensorTime getSensorTime(const Time& time) const
    { return SensorTime(sensor(), time + mTimeOffset); }

private Q_SLOTS:
  void onBufferCompleted(bool failed);
  void onNewDataEnd(const ObsData_pv& data);
  void onUpdateDataEnd(const ObsData_pv& data);
  void onDropDataEnd(const SensorTime_v& dropped);

protected:
  EditAccess_p mDA;
  TimeBuffer_p mBuffer;
  DataItem_p mItem;
  bool mHeaderShowStation;
  boost::posix_time::time_duration mTimeOffset;
  bool mRequestBusy;
};

HQC_TYPEDEF_P(DataColumn);

#endif // DATACOLUMN_HH
