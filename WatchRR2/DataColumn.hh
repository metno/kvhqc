
#ifndef DATACOLUMN_HH
#define DATACOLUMN_HH 1

#include "DataItem.hh"
#include "EditAccess.hh"
#include "ObsColumn.hh"

class DataColumn : public ObsColumn {
public:
    DataColumn(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, DataItemPtr item);
    ~DataColumn();

    void setHeaderShowStation(bool show)
        { mHeaderShowStation = show; }

    virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
    virtual QVariant data(const timeutil::ptime& time, int role) const;
    virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);
    virtual QVariant headerData(Qt::Orientation orientation, int role) const;

    DataItemPtr getItem() const
        { return mItem; }
    virtual bool matchSensor(const Sensor& sensorObs) const;

    virtual const boost::posix_time::time_duration& timeOffset() const
        { return mTimeOffset; }
    void setTimeOffset(const boost::posix_time::time_duration& timeOffset);

protected:
    EditDataPtr getObs(const timeutil::ptime& time) const;
    SensorTime getSensorTime(const timeutil::ptime& time) const
        { return SensorTime(mSensor, time + mTimeOffset); }

    virtual bool onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);

protected:
    EditAccessPtr mDA;
    Sensor mSensor;
    TimeRange mTime;
    DataItemPtr mItem;
    bool mHeaderShowStation;
    boost::posix_time::time_duration mTimeOffset;

    typedef std::map<timeutil::ptime, EditDataPtr> ObsCache_t;
    mutable ObsCache_t mObsCache;
};

typedef boost::shared_ptr<DataColumn> DataColumnPtr;

#endif // DATACOLUMN_HH
