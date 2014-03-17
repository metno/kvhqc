
#ifndef ACCESSCOLUMN_HH
#define ACCESSCOLUMN_HH 1

#include "ObsAccess.hh"
#include "TimeBuffer.hh"

#include "common/DataItem.hh"
#include "common/ObsColumn.hh"

class AccessColumn : public QObject, public ObsColumn
{ Q_OBJECT;

public:
  AccessColumn(ObsAccess_p oa, const Sensor& sensor, const TimeRange& time, DataItemPtr item);
  ~AccessColumn();

  void setHeaderShowStation(bool show)
    { mHeaderShowStation = show; }

  virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
  virtual QVariant data(const Time& time, int role) const;
  virtual bool setData(const Time& time, const QVariant& value, int role);
  virtual QVariant headerData(Qt::Orientation orientation, int role) const;

  virtual bool matchSensor(const Sensor& sensorObs) const;

  virtual const boost::posix_time::time_duration& timeOffset() const
    { return mTimeOffset; }

  virtual Sensor sensor() const
    { return *mBuffer->request()->sensors().begin(); }

  virtual int type() const
    { return mItem->type(); }

protected:
  ObsData_p getObs(const Time& time) const;
  SensorTime getSensorTime(const Time& time) const
    { return SensorTime(sensor(), time + mTimeOffset); }

private Q_SLOTS:
  void onBufferCompleted(bool failed);
  void onNewDataEnd(const ObsData_pv& data);
  void onUpdateDataEnd(const ObsData_pv& data);
  void onDropDataEnd(const SensorTime_v& dropped);

protected:
  ObsAccess_p mDA;
  TimeBuffer_p mBuffer;
  DataItemPtr mItem;
  bool mHeaderShowStation;
  boost::posix_time::time_duration mTimeOffset;
};

HQC_TYPEDEF_P(AccessColumn);

#endif // ACCESSCOLUMN_HH
