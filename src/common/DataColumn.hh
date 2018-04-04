
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

  void attach(ObsTableModel* table) override;
  void detach(ObsTableModel* table) override;

  virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
  virtual QVariant data(const timeutil::ptime& time, int role) const;
  virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);
  virtual QVariant headerData(Qt::Orientation orientation, int role) const;

  DataItem_p getItem() const
    { return mItem; }
  virtual bool matchSensor(const Sensor& sensorObs) const;

  void setTimeOffset(const boost::posix_time::time_duration& timeOffset);

  void setTimeSpan(const TimeSpan& tr);

  virtual const Sensor& sensor() const;
  virtual Time_s times() const;
  virtual int type() const
    { return mItem->type(); }

  virtual bool isBusy() const
    { return mRequestBusy; }

  ObsData_p getObs(const Time& time) const;

private:
  const boost::posix_time::time_duration& timeOffset() const
    { return mTimeOffset; }

  bool hasTimeOffset() const { return timeOffset().total_seconds() != 0; }

  SensorTime sensorTimeC2B(const Time& time) const
    { return SensorTime(sensor(), timeC2B(time)); }

  //! Convert buffer time to column time.
  /*! Subtracts timeOffset */
  Time timeB2C(const Time& time) const
    { return time - timeOffset(); }

  //! Convert column time to buffer time.
  /*! Adds timeOffset */
  Time timeC2B(const Time& time) const
    { return time + timeOffset(); }

  void makeBuffer();

private Q_SLOTS:
  void onBufferCompleted(const QString&);
  void onNewDataEnd(const ObsData_pv& data);
  void onUpdateDataEnd(const ObsData_pv& data);
  void onDropDataEnd(const SensorTime_v& dropped);

protected:
  EditAccess_p mDA;
  Sensor mSensor;
  TimeSpan mTimeSpan;
  TimeBuffer_p mBuffer;
  DataItem_p mItem;
  bool mAttached;
  bool mHeaderShowStation;
  boost::posix_time::time_duration mTimeOffset;
  bool mRequestBusy;
};

HQC_TYPEDEF_P(DataColumn);

#endif // DATACOLUMN_HH
